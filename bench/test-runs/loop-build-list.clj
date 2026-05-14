; Build 100.000 element list in a loop
(loop [i 100000
       lst ()]
  (when (> i 0)
    (recur (dec i) (cons i lst))))
