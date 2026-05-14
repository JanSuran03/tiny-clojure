; 2^21; recursive; no caching
; predefs start
(defn compute-exp [power]
  (if (= power 0)
    1
    (+ (compute-exp (dec power))
       (compute-exp (dec power)))))
; predefs end
(compute-exp 19)
