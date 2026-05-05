#!/usr/bin/env bb

(println "Running benchmark...")

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
  (if (= exit 0)
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
  (let [form-with-predefs (str clojure-predefs "\n" common-predefs "\n" clojure-form)]
    (ensure-successful-run
     (sh/sh "clojure" "--repl" :in form-with-predefs)
     "Clojure")))

(defn run-tinyclj
  ([clojure-form extra-opts]
   (let [form-with-predefs (str common-predefs "\n"
                                (replace-for-tinyclj clojure-form))
         sh-args (concat [(str tinyclj-exe) "--suppress-repl-welcome"]
                         extra-opts
                         [:in (replace-for-tinyclj form-with-predefs)])]
     (ensure-successful-run
      (apply sh/sh sh-args)
      "TinyClojure"))))

(def run-configs
  (for [opt-level ["O0" "O2"]
        #_todo]
    {:opt-level opt-level}))

(def predefs-start "; predefs start")
(def predefs-end "; predefs end")

(defn extract-line [s]
  (str/split s #"\r?\n" 2))

(defn parse-bench-input [input]
  (let [[test-name input] (str/split input #"\r?\n" 2)]
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

(defn process-benchmark-output [output]
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
     :std-dev std-dev
     :num-runs (count times)}))

(defn report-benchmark [test-name config clj-output tinyclj-output]
  (let [{clj-avg     :average-time
         clj-std-dev :std-dev
         run-runs    :num-runs
         :as         clj-bench-report} (process-benchmark-output clj-output)
        clj-std-dev% (if (zero? clj-avg)
                       0.0
                       (* 100.0 (/ clj-std-dev clj-avg)))
        {tinyclj-avg     :average-time
         tinyclj-std-dev :std-dev
         :as             tinyclj-bench-report} (process-benchmark-output tinyclj-output)
        tinyclj-std-dev% (if (zero? tinyclj-avg)
                           0.0
                           (* 100.0 (/ tinyclj-std-dev tinyclj-avg)))]
    (println (format (str "Benchmark '%s' with config %s:\n"
                          "  Number of runs       = %d,\n"
                          "  Clojure:\n"
                          "    Average time       = %.4f ms,\n"
                          "    Standard deviation = %.4f ms (~%.2f %%),\n"
                          "  TinyClojure:\n"
                          "    Average time       = %.4f ms,\n"
                          "    Standard deviation = %.4f ms (~%.2f %%)\n")
                     test-name
                     (str config)
                     run-runs
                     clj-avg clj-std-dev clj-std-dev%
                     tinyclj-avg tinyclj-std-dev tinyclj-std-dev%))))

(defn run-benchmark [input-file opts]
  (let [processed-opts (mapcat (fn [[k v]]
                                 [(str "-" (name k)) v])
                               opts)
        input (slurp (str input-file))
        input-file-name (fs/file-name input-file)
        [test-name input predefs] (parse-bench-input input)
        test-name (subs test-name 2) ; drop the leading "; "
        wrapped-input (str predefs "\n"
                           "(bench-and-report " input ")")
        {tinyclj-out :out} (run-tinyclj wrapped-input processed-opts)
        {clojure-out :out} (run-clojure wrapped-input)]
    (report-benchmark test-name opts clojure-out tinyclj-out)))

; clear the dump directory first
(fs/delete-tree dump-dir)
; run all benchmarks now
(doseq [test-file (fs/list-dir test-run-directory)
        config run-configs]
  (run-benchmark test-file config))
