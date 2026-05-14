(let [f (fn
          ([a b c]
           (+ a b c))
          ([a b & rest]
            (+ a b)))]
  (f 1 2))