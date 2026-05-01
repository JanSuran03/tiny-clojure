#!/usr/bin/env bb

(require '[clojure.string :as str]
         '[clojure.java.io :as io]
         '[babashka.fs :as fs]
         '[clojure.java.shell :as sh])

(def script-dir (fs/parent *file*))
(def project-root (fs/parent script-dir))
(def forms-path (fs/path script-dir "forms.clj"))
(def error-files-directory (fs/path script-dir "error-files"))
(def tinyclj-exe (fs/path project-root "cmake-build-debug" "tiny-clojure"))

(defn trim-clojure-repl-output [output]
  ; Clojure REPL outputs the Clojure version "Clojure 1.11.1" and similar on the first line => skip this line
  (second (str/split output #"\r?\n" 2)))

(defn replace-for-tinyclj [form]
  (-> form
      (str/replace #"\[" "(")
      (str/replace #"\]" ")")))

(defn ensure-successful-run [{:keys [exit our err] :as result} command-name]
  (if (= exit 0)
    result
    (do (println (str "Error: " command-name " execution failed with exit code " exit "!"))
        (when-not (str/blank? our)
          (println command-name " Output:")
          (println our))
        (when-not (str/blank? err)
          (println command-name " Error Output:")
          (println err))
        (System/exit 1))))

(defn run-clojure [clojure-form]
  (ensure-successful-run
    (sh/sh "clojure" "--repl" :in clojure-form)
    "Clojure"))

(defn run-tinyclj
  ([clojure-form] (run-tinyclj clojure-form {:no-transform false}))
  ([clojure-form {:keys [no-transform]}]
   (cond-> clojure-form
     (not no-transform) replace-for-tinyclj
     true (#(sh/sh (str tinyclj-exe) "--suppress-repl-welcome" :in %))
     true (ensure-successful-run "TinyClojure"))))

(defn run-standard-tests []
  (println "Running standard tests...")
  (let [forms-path (fs/path script-dir "forms.clj")
        tinyclj-exe (fs/path project-root "cmake-build-debug" "tiny-clojure")
        forms (slurp (str forms-path))
        {clojure-out :out clojure-err :err} (run-clojure forms)
        {tinyclj-out :out tinyclj-err :err} (run-tinyclj forms)
        clojure-out (trim-clojure-repl-output clojure-out)]
    (cond
      (or (not (str/blank? clojure-err))
          (not (str/blank? tinyclj-err)))
      (do (println "Error: Unexpected error in Clojure or TinyClojure execution!")
          (when-not (str/blank? clojure-err)
            (println "Clojure Error Output:")
            (println clojure-err))
          (when-not (str/blank? tinyclj-err)
            (println "TinyClojure Error Output:")
            (println tinyclj-err))
          (System/exit 1))

      (= clojure-out tinyclj-out)
      (println "Standard tests succeeded! Clojure and TinyClojure outputs match.")

      :else
      (do
        (println "Clojure and TinyClojure outputs differ!")
        ; also compare line-by-line
        (let [clojure-lines (str/split-lines clojure-out)
              tinyclj-lines (str/split-lines tinyclj-out)]
          (if-not (= (count clojure-lines) (count tinyclj-lines))
            (do (println "Number of lines differ!")
                (println "Clojure Lines:" (count clojure-lines))
                (println "TinyClojure Lines:" (count tinyclj-lines))
                (println "Full Clojure Output:")
                (println clojure-out)
                (println "Full TinyClojure Output:")
                (println tinyclj-out))
            (doseq [[i [clojure-line tinyclj-line]] (->> (map vector clojure-lines tinyclj-lines)
                                                         (map-indexed vector))]
              (when-not (= clojure-line tinyclj-line)
                (println (str "Difference at line " (inc i) ":"))
                (println "Clojure:    " clojure-line)
                (println "TinyClojure:" tinyclj-line)))))
        (System/exit 1)))))

(defn run-failing-tests []
  (println "Running failing tests...")
  (let [input-files (fs/list-dir error-files-directory)
        test-results (map (fn [error-file]
                            (let [form (slurp (str error-file))]
                              ; if a form start with "; tinyclj-only", then only check that it fails in tinyclj,
                              ; otherwise check that it fails in both clojure and tinyclj
                              (if (str/starts-with? form "; tinyclj-only")
                                (let [{tinyclj-err :err} (run-tinyclj form {:no-transform true})]
                                  (when (str/blank? tinyclj-err)
                                    :no-tinyclj-error))
                                (let [{clojure-err :err} (run-clojure form)
                                      {tinyclj-err :err} (run-tinyclj form)]
                                  (case [(boolean (seq clojure-err)) (boolean (seq tinyclj-err))]
                                    [true true] nil ; both failed as expected
                                    [true false] :no-tinyclj-error
                                    [false true] :no-clojure-error
                                    [false false] :no-tinyclj-and-clojure-error)))))
                          input-files)
        failing-cases (->> (map (fn [file res]
                                  {:file file :res res})
                                input-files test-results)
                           (filter :res))]
    (if (empty? failing-cases)
      (println "All" (count input-files) "error files failed as expected!")
      (do (println "The following error files did not fail as expected:")
          (doseq [{:keys [file res]} failing-cases]
            (println " - " (fs/file-name file) ":" res))
          (System/exit 1)))))

(run-standard-tests)
(run-failing-tests)
