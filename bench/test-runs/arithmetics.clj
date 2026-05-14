; iterative sum 1 to 100.000
(loop [i 100000
       sum 0]
  (if (> i 0)
    (recur (dec i) (+ sum i))
    sum))