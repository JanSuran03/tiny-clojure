#!/usr/bin/env bb

(println "Running tests...")

(require '[clojure.string :as str]
         '[clojure.java.io :as io]
         '[babashka.fs :as fs]
         '[clojure.java.shell :as sh])

(defn trim-clojure-repl-output [output]
  ; Clojure REPL outputs the Clojure version "Clojure 1.11.1" and similar on the first line => skip this line
  (second (str/split output #"\r?\n" 2)))

(let [script-dir (fs/parent *file*)
      project-root (fs/parent script-dir)
      forms-path (fs/path script-dir "forms.clj")
      tinyclj-exe (fs/path project-root "cmake-build-debug" "tiny-clojure")
      clojure-forms (-> (str forms-path)
                        slurp)
      tinyclj-forms (-> clojure-forms (str/replace #"\[" "(")
                        (str/replace #"\]" ")"))
      {clojure-out :out} (sh/sh "clojure" "--repl" :in clojure-forms)
      {tinyclj-out :out} (sh/sh (str tinyclj-exe) "--suppress-repl-welcome" :in tinyclj-forms)
      clojure-out (trim-clojure-repl-output clojure-out)]
  (if (= clojure-out tinyclj-out)
    (println "Tests succeeded! Clojure and TinyClojure outputs match.")
    (do
      (println "Clojure and TinyClojure outputs differ!")
      ; also compare line-by-line
      (let [clojure-lines (str/split clojure-out #"\r?\n")
            tinyclj-lines (str/split tinyclj-out #"\r?\n")]
        (if-not (= (count clojure-lines) (count tinyclj-lines))
          (do (println "Number of lines differ!")
              (println "Clojure Lines:" (count clojure-lines))
              (println "TinyClojure Lines:" (count tinyclj-lines)))
          (doseq [[i [clojure-line tinyclj-line]] (->> (map vector clojure-lines tinyclj-lines)
                                                       (map-indexed vector))]
            (when-not (= clojure-line tinyclj-line)
              (println (str "Difference at line " (inc i) ":"))
              (println "Clojure:    " clojure-line)
              (println "TinyClojure:" tinyclj-line)))))
      (System/exit 1))))
