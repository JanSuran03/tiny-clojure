(let [f (fn
          ([a & rest]
           a)
          ([a b & rest]
            b))]
  (f 1 2))