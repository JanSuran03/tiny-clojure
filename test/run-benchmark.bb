#!/usr/bin/env bb

(println "Running benchmark...")

(require '[clojure.string :as str]
         '[clojure.java.io :as io]
         '[babashka.fs :as fs]
         '[clojure.java.shell :as sh])

(defn trim-clojure-repl-output [output]
  ; Clojure REPL outputs the Clojure version "Clojure 1.11.1" and similar on the first line => skip this line
  (second (str/split output #"\r?\n" 2)))

(let [script-dir (fs/parent *file*)
      project-root (fs/parent script-dir)
      forms-path (fs/path script-dir "benchmark-forms.clj")
      bench-predefs-path (fs/path script-dir "bench-predefs.clj")
      tinyclj-exe (fs/path project-root "cmake-build-debug" "tiny-clojure")
      bench-predefs (-> (str bench-predefs-path)
                        slurp)
      original-forms (-> (str forms-path)
                        slurp)
      clojure-forms (str bench-predefs "\n" original-forms)
      tinyclj-forms (-> original-forms (str/replace #"\[" "(")
                        (str/replace #"\]" ")"))
      {clojure-out :out} (sh/sh "clojure" "--repl" :in clojure-forms)
      {tinyclj-out :out} (sh/sh (str tinyclj-exe) "--suppress-repl-welcome" :in tinyclj-forms)
      clojure-out (trim-clojure-repl-output clojure-out)]
  (println "Clojure Output:\n" clojure-out)
  (println "TinyClojure Output:\n" tinyclj-out))
