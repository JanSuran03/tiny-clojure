(defmacro bench-and-report [form]
  `(do (println "--bench start--")
       (dotimes [i# {{num-runs}}]
         (let [time# (bench ~form)]
           (println time#)))
       (println "--bench end--")))