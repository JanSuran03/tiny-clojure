(let [f (fn [g] (g 42))]
  (f nil)
  (println "Hello"))