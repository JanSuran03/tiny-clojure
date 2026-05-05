; Loop: build 200.000 element list
(loop [i 200000
       lst ()]
  (when (> i 0)
    (recur (dec i) (cons i lst))))
