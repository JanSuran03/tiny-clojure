(defmacro side-effect-macro (form)
  (println "Macro called")
  form)

(def hello "hello")

(defn world ()
  (side-effect-macro "world"))

(println hello (world))