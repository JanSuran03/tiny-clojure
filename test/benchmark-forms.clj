(bench
  (loop [i 1000000]
    (when (> i 0)
      (recur (dec i)))))
