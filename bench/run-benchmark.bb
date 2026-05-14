#!/usr/bin/env bb

(println "Running benchmarks...\n")

(require '[clojure.string :as str]
         '[clojure.java.io :as io]
         '[babashka.fs :as fs]
         '[clojure.java.shell :as sh]
         '[clojure.data.csv :as csv]
         '[selmer.parser :as selmer])

(defn trim-clojure-repl-output [output]
  ; Clojure REPL outputs the Clojure version "Clojure 1.11.1" and similar on the first line => skip this line
  (second (str/split output #"\r?\n" 2)))

(def script-dir (fs/parent *file*))
(def project-root (fs/parent script-dir))
(def tinyclj-exe (fs/path project-root "cmake-build-debug" "tiny-clojure"))
(def clojure-predefs-path (fs/path script-dir "clj-bench-predefs.clj"))
(def common-predefs-path (fs/path script-dir "common-bench-predefs.clj"))
(def test-run-directory (fs/path script-dir "test-runs"))
(def dump-dir (fs/path script-dir "dump"))

(def clojure-predefs (-> (str clojure-predefs-path)
                         slurp))
(def common-predefs (-> (str common-predefs-path)
                        slurp))

(defn ensure-successful-run [{:keys [exit out err] :as result} command-name]
  (if (and (= exit 0) (str/blank? err))
    result
    (do (println (str "Error: " command-name " execution failed with exit code " exit "!"))
        (when-not (str/blank? out)
          (println command-name " Output:")
          (println out))
        (when-not (str/blank? err)
          (println command-name " Error Output:")
          (println err))
        (System/exit 1))))

(defn replace-for-tinyclj [form]
  (-> form
      (str/replace #"\[" "(")
      (str/replace #"\]" ")")))

(defn run-clojure [clojure-form]
  (ensure-successful-run
   (sh/sh "clojure" "--repl" :in clojure-form)
   "Clojure"))

(defn run-tinyclj
  ([clojure-form extra-opts]
   (let [replaced-form (replace-for-tinyclj clojure-form)
         sh-args (concat [(str tinyclj-exe) "--suppress-repl-welcome"]
                         extra-opts
                         [:in (replace-for-tinyclj replaced-form)])]
     (ensure-successful-run
      (apply sh/sh sh-args)
      "TinyClojure"))))

(def run-configs
  (for [#_#_opt-level ["O0" "O2"]
        #_#_direct-linking [false true]
        int-cache-range ["off"
                         "-1:1"
                         "-2:2"
                         "-4:4"
                         "-8:8"
                         "-16:16"]]
    {:opt-level "O2" #_opt-level
     :direct-linking true #_direct-linking
     :int-cache-range int-cache-range}))

(def predefs-start "; predefs start")
(def predefs-end "; predefs end")

(defn extract-line [s]
  (str/split s #"\r?\n" 2))

; test-name maybe-predefs? form
(defn parse-bench-input [input]
  (let [[test-name input] (str/split input #"\r?\n" 2)
        ; drop the leading "; "
        test-name (subs test-name 2)]
    (if (str/starts-with? input predefs-start)
      (let [[_ input] (extract-line input)
            end-index (str/index-of input predefs-end)
            _ (when-not end-index
                (throw (ex-info "Predefs end marker not found in input!" {:input input})))
            predefs (subs input 0 end-index)
            input (subs input (+ end-index (count predefs-end)))]
        [test-name input predefs])
      [test-name input nil])))

(defn parse-benchmark-output [output]
  (let [lines (str/split output #"\r?\n")]
    (->> lines
         (drop-while #(not (str/includes? % "--bench start--")))
         next
         (take-while #(not (str/includes? % "--bench end--"))))))

(defn process-benchmark [output]
  (let [filtered-output-lines (parse-benchmark-output output)
        times (map #(Double/parseDouble %) filtered-output-lines)
        ; drop the first 2 runs: JIT warmup & optimizations
        #_#_times (drop 1 times)
        ; calculate the average of the remaining runs
        average-time (double (/ (reduce + times) (count times)))
        ; calculate the standard deviation
        squared-diffs (map #(Math/pow (- % average-time)
                                      2)
                           times)
        std-dev (Math/sqrt (/ (reduce + squared-diffs)
                              (count times)))]
    {:average-time average-time
     :std-dev      std-dev
     :num-runs     (count times)}))

(defn run-with-clojure [input]
  (let [wrapped-input (str clojure-predefs "\n" input)]
    (:out (run-clojure wrapped-input))))

(defn run-benchmark [input config]
  (let [processed-opts (mapcat (fn [[k v]]
                                 [(str "-" (name k)) (str v)])
                               config)]
    (:out (run-tinyclj input processed-opts))))

(defn column-width [results index]
  (->> results (map #(nth % index))
       (map count)
       (apply max)))

(defn str-of [width char]
  (apply str (repeat width char)))

(defn blank-str [width]
  (str-of width " "))

(defn center-label [label width]
  (let [padding (max 0 (- width (count label)))
        left-padding (Math/floor (/ padding 2))
        right-padding (- padding left-padding)]
    (str (blank-str left-padding) label (blank-str right-padding))))

(def csv-columns [:benchmark
                  :implementation
                  :opt-level
                  :direct-linking
                  :int-cache-range
                  :avg-ms
                  :std-dev-ms
                  :std-dev-pct
                  :slowdown-vs-clojure])

(def csv-rows (atom []))

(defn run-benchmarks [input-file configs num-tests]
  (let [input (slurp (str input-file))
        [test-name input predefs] (parse-bench-input input)
        common-predefs (selmer/render common-predefs {:num-runs num-tests})
        input-with-predefs (str common-predefs "\n"
                                predefs "\n"
                                "(bench-and-report " input ")")
        clojure-out (run-with-clojure input-with-predefs)
        {clj-avg     :average-time
         clj-std-dev :std-dev
         :keys       [num-runs]
         :as         clj-res} (process-benchmark clojure-out)
        clj-std-dev% (if (zero? clj-avg)
                       0.0
                       (* 100.0 (/ clj-std-dev clj-avg)))
        reports (map #(-> (run-benchmark input-with-predefs %)
                          process-benchmark)
                     configs)]
    (swap! csv-rows conj {:benchmark test-name
                          :implementation "JVM Clojure"
                          :opt-level "N/A"
                          :direct-linking "N/A"
                          :int-cache-range "N/A"
                          :avg-ms clj-avg
                          :std-dev-ms clj-std-dev
                          :std-dev-pct clj-std-dev%
                          :slowdown-vs-clojure 1.0})
    (doseq [[{:keys [opt-level direct-linking int-cache-range] :as config} {:keys [average-time std-dev] :as report}]
            (map vector configs reports)

            :let [avg-time-fmt (format "%.4f ms" average-time)
                  std-dev-pct (if (zero? average-time)
                                0.0
                                (* 100 (/ std-dev average-time)))
                  slowdown-ratio (if (zero? clj-avg)
                                   0.0
                                   (/ average-time clj-avg))]]
      (swap! csv-rows conj {:benchmark test-name
                            :implementation "TinyClojure"
                            :opt-level opt-level
                            :direct-linking direct-linking
                            :int-cache-range int-cache-range
                            :avg-ms average-time
                            :std-dev-ms std-dev
                            :std-dev-pct clj-std-dev%
                            :slowdown-vs-clojure slowdown-ratio}))))

(defn write-csv! [path rows]
  (with-open [writer (io/writer (str path))]
    (csv/write-csv writer
                   (cons (map name csv-columns)
                         (map (fn [row]
                                (map #(get row %) csv-columns))
                              rows))))
  (reset! csv-rows []))

; clear the dump directory first
(fs/delete-tree dump-dir)
(fs/create-dirs dump-dir)

(defn filter-files [files s]
  (filter #(str/includes? (str %) s) files))

; focus on integer cache ranges
(let [configs (for [int-cache-range ["off"
                                     "-1:1"
                                     "-2:2"
                                     "-4:4"
                                     "-8:8"
                                     "-16:16"]]
                {:opt-level       "O2"
                 :direct-linking  true
                 :int-cache-range int-cache-range})]
  (doseq [test-file (fs/list-dir test-run-directory)]
    (run-benchmarks test-file configs 15)))

(write-csv! (fs/path dump-dir "integer-caching.csv") @csv-rows)

; focus on LLVM optimizations; keep the default integer cache range & direct linking
(let [configs (for [opt-level ["O0" "O2"]]
                {:opt-level      opt-level
                 :direct-linking true})]
  (doseq [test-file (fs/list-dir test-run-directory)]
    (run-benchmarks test-file configs 15)))

(write-csv! (fs/path dump-dir "llvm-optimizations.csv") @csv-rows)

; focus on direct linking optimizations; keep the default integer cache range & LLVM optimizations
(let [configs (for [direct-linking [false true]]
                {:direct-linking direct-linking})]
  (doseq [test-file (fs/list-dir test-run-directory)]
    (run-benchmarks test-file configs 15)))

(write-csv! (fs/path dump-dir "direct-linking.csv") @csv-rows)




