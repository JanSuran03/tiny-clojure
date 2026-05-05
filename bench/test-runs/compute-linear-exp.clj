; Compute 2^21 using linear branching
; predefs start
(defn compute-exp [power]
  (if (= power 0)
    1
    (+ (compute-exp (dec power))
       (compute-exp (dec power)))))
; predefs end
(compute-exp 21)
