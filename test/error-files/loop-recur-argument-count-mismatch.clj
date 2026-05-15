(loop [a 1 b 2]
  (when (> a b)
    (recur a)))