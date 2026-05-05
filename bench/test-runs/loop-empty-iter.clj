; Loop: empty iter 200.000 times
(loop [i 200000]
  (when (> i 0)
    (recur (dec i))))
