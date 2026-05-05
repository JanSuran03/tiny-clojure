#!/usr/bin/env bb

(println "Running benchmarks...\n")

(require '[clojure.string :as str]
         '[clojure.java.io :as io]
         '[babashka.fs :as fs]
         '[clojure.java.shell :as sh])

(defn trim-clojure-repl-output [output]
  ; Clojure REPL outputs the Clojure version "Clojure 1.11.1" and similar on the first line => skip this line
  (second (str/split output #"\r?\n" 2)))

(def script-dir (fs/parent *file*))
(def project-root (fs/parent script-dir))
(def tinyclj-exe (fs/path project-root "cmake-build-debug" "tiny-clojure"))
(def clojure-predefs-path (fs/path script-dir "clj-bench-predefs.clj"))
(def common-predefs-path (fs/path script-dir "common-bench-predefs.clj"))
(def test-run-directory (fs/path script-dir "test-runs"))
(def dump-dir "dump")

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
  (for [opt-level ["O0" "O2"]
        direct-linking [false true]]
    {:opt-level      opt-level
     :direct-linking direct-linking}))

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
        times (drop 1 times)
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

(defn format-benchmark-report [{:keys [average-time std-dev]} clojure-time]
  (let [tinyclj-clj-ratio (if (zero? clojure-time)
                            0.0
                            (/ average-time clojure-time))
        std-dev% (if (zero? average-time)
                   0.0
                   (* 100 (/ std-dev average-time)))]
    [(format "%.4f ms" average-time)
     (format "%.4f ms (~%.2f %%)" std-dev std-dev%)
     (format "%.2fx" tinyclj-clj-ratio)]))

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

(defn run-benchmarks [input-file configs]
  (let [input (slurp (str input-file))
        [test-name input predefs] (parse-bench-input input)
        input-with-predefs (str common-predefs "\n"
                                predefs "\n"
                                "(bench-and-report " input ")")
        clojure-out (run-with-clojure input-with-predefs)
        {clj-avg     :average-time
         clj-std-dev :std-dev
         :keys       [num-runs]
         :as         clj-res} (process-benchmark clojure-out)
        formatted-clj-report (format-benchmark-report clj-res clj-avg)
        clj-std-dev% (if (zero? clj-avg)
                       0.0
                       (* 100.0 (/ clj-std-dev clj-avg)))
        formatted-reports (map #(-> (run-benchmark input-with-predefs %)
                                    process-benchmark
                                    (format-benchmark-report clj-avg))
                               configs)
        ; for printing the report in a tabular format
        formatted-reports' (list* ["Average Time" "Standard deviation" "Slowdown vs Clojure"]
                                  formatted-clj-report
                                  formatted-reports)
        configs' (list* "Config" "JVM Clojure" (map str configs))
        max-config-len (->> configs (map str) (map count) (apply max))
        max-time-len (column-width formatted-reports' 0)
        max-std-dev-len (column-width formatted-reports' 1)
        max-ratio-len (column-width formatted-reports' 2)
        sep-str (str "+-" (str-of max-config-len "-") "-+-"
                     (str-of max-time-len "-") "-+-"
                     (str-of max-std-dev-len "-") "-+-"
                     (str-of max-ratio-len "-") "-+\n")]
    ; print the benchmark header
    (printf "=== Benchmark: '%s' (%d iterations) === \n" test-name num-runs)
    (doseq [[config [avg-time std-dev slowdown-ratio]] (map vector configs' formatted-reports')
            :let [config-padding (- max-config-len (count (str config)))
                  avg-pading  (- max-time-len (count avg-time))
                  std-dev-padding  (- max-std-dev-len (count std-dev))
                  ratio-padding (- max-ratio-len (count slowdown-ratio))]]
      (print sep-str)
      (print (str "| " (center-label config max-config-len) " | "
                  (center-label avg-time max-time-len) " | "
                  (center-label std-dev max-std-dev-len) " | "
                  (center-label slowdown-ratio max-ratio-len) " |\n")))
    ; flush the entire report for this benchmark before printing the next one
    (println sep-str "\n")))

; clear the dump directory first
(fs/delete-tree dump-dir)

(doseq [test-file (fs/list-dir test-run-directory)]
  (run-benchmarks test-file run-configs))
