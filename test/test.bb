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

(defn run-clojure [clojure-form]
  (ensure-successful-run
   (sh/sh "clojure" "--repl" :in clojure-form)
   "Clojure"))

(defn run-tinyclj
  ([clojure-form] (run-tinyclj clojure-form {:no-transform false}))
  ([clojure-form {:keys [no-transform] :as options}]
   (let [default-args {:suppress-repl-welcome true
                       :compiled-dir          "test-bb"}
         additional-args (dissoc options :no-transform)
         args (merge default-args additional-args)
         args-seq (->> args
                       (mapcat (fn [[k v]]
                                 [(str "-" (name k)) (str v)])))
         transformed-form (if no-transform
                            clojure-form
                            (replace-for-tinyclj clojure-form))
         sh-args (concat [(str tinyclj-exe)] args-seq [:in transformed-form])]
     (ensure-successful-run
      (apply sh/sh sh-args)
      "TinyClojure"))))

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

(def aot-root (fs/path script-dir "aot"))
(def aot-dump-dir (fs/path aot-root "dump"))

(defn run-aot-tests []
  ; Clear the copiled dirs!
  (println "Running AOT tests...")
  (fs/delete-tree aot-dump-dir)
  (fs/create-dirs aot-dump-dir)

  (let [run-1-input "(compile-module \"example\")"
        run-2-input "(load-module \"example\")"
        macro-output "Macro called"
        common-output "hello world"
        ; 1) Run TinyClojure with the 1. input and check that the macro output and then the common output are printed
        ; 2) Run TinyClojure with the 2. input and check that only the common output is printed
        {run-1-out :out run-1-err :err} (run-tinyclj run-1-input {:no-transform  true
                                                                  :compiled-dir  (str aot-dump-dir)
                                                                  :user-code-dir (str aot-root)})
        {run-2-out :out run-2-err :err} (run-tinyclj run-2-input {:no-transform  true
                                                                  :compiled-dir  (str aot-dump-dir)
                                                                  :user-code-dir (str aot-root)})]
    (cond (or (not (str/blank? run-1-err))
              (not (str/blank? run-2-err)))
          (do (println "Error: Unexpected error in TinyClojure execution!")
              (when-not (str/blank? run-1-err)
                (println "Run 1 Error Output:")
                (println run-1-err))
              (when-not (str/blank? run-2-err)
                (println "Run 2 Error Output:")
                (println run-2-err))
              (System/exit 1))

          (and (str/includes? run-1-out macro-output)
               (str/includes? run-1-out common-output)
               (not (str/includes? run-2-out macro-output))
               (str/includes? run-2-out common-output))
          (println "AOT tests succeeded! Macro output and common output are as expected in both runs.")

          :else
          (do (println "AOT test outputs did not match expectations!")
              (println (str "The expected behavior is that the first run should include\n"
                            "both the macro output and the common output, while the second run\n"
                            "should only include the common output without the macro output."))
              (println "Run 1 Output:")
              (println run-1-out)
              (println "Run 2 Output:")
              (println run-2-out)
              (System/exit 1)))))

(run-standard-tests)
(newline)
(run-failing-tests)
(newline)
(run-aot-tests)
(newline)
