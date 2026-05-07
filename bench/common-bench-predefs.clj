(defmacro bench-and-report [form]
  `(do (println "--bench start--")
       (dotimes [i# 15]
         (let [time# (bench ~form)]
           (println time#)))
       (println "--bench end--")))