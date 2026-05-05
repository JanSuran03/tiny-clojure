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

(defn replace-for-tinyclj [form]
  (-> form
      (str/replace #"\[" "(")
      (str/replace #"\]" ")")))

(def run-configs
  (for [opt-level ["O0" "O2"]
        #_todo]
    {:opt-level opt-level}))

(defn report-benchmark [test-name config repl-output]
  (let [repl-output-lines (str/split repl-output #"\r?\n")
        filtered-output-lines (->> repl-output-lines
                                   (drop-while #(not= % "user=> --bench start--"))
                                   next
                                   (take-while #(not= % "--bench end--")))
        times (map #(Double/parseDouble %) filtered-output-lines)
        ; drop the first 2 runs: JIT warmup & optimizations
        times (drop 2 times)
        ; calculate the average of the remaining runs
        average-time (double (/ (reduce + times) (count times)))
        ; calculate the standard deviation
        squared-diffs (map #(Math/pow (- % average-time)
                                      2)
                           times)
        std-dev (Math/sqrt (/ (reduce + squared-diffs)
                              (count times)))]
    (println (format (str "Benchmark '%s' with config %s:\n"
                          "  Number of runs     = %d,\n"
                          "  Average time       = %.4f ms,\n"
                          "  Standard deviation = %.4f ms")
                     test-name
                     (str config)
                     (count times)
                     average-time
                     std-dev))))

(defn run-benchmark [input-file opts]
  (let [processed-opts (mapcat (fn [[k v]]
                                 [(str "-" (name k)) v])
                               opts)
        input (slurp (str input-file))
        input-file-name (fs/file-name input-file)
        [test-name input] (str/split input #"\r?\n" 2)
        test-name (subs test-name 2) ; drop the leading "; "
        wrapped-input (str "(bench-and-report " input ")")
        input-with-predefs (str common-predefs "\n" (replace-for-tinyclj wrapped-input))
        sh-args (concat [(str tinyclj-exe) "--suppress-repl-welcome"]
                        processed-opts
                        ["-compiled-dir" (str (fs/path script-dir dump-dir input-file-name))]
                        [:in input-with-predefs])
        {:keys [out err exit]} (apply sh/sh sh-args)]
    (if (or (= exit 0) (not (str/blank? err)))
      (report-benchmark test-name opts out)
      (do (println "Error: TinyClojure execution failed with exit code " exit "!")
          (when-not (str/blank? out)
            (println "TinyClojure Output:")
            (println out))
          (when-not (str/blank? err)
            (println "TinyClojure Error Output:")
            (println err))
          (System/exit 1)))))

; clear the dump directory first
(fs/delete-tree dump-dir)
; run all benchmarks now
(doseq [test-file (fs/list-dir test-run-directory)
        config run-configs]
  (run-benchmark test-file config))
