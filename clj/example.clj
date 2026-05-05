(defn add (a b)
  (+ a b))

(defmacro print-form (form)
  (println "Macroexpanding" form)
  form)

(println (+ 1 (print-form 2) 3))

(def const-c 5)

(println (str "Hello, " const-c "!"))
