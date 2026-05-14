; fib(27); recursive
; predefs start
(defn fib [n]
  (if (<= n 1)
    n
    (+ (fib (dec n))
       (fib (- n 2)))))
; predefs end
(fib 27)
