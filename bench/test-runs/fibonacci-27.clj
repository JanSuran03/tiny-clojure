; Loop: Fibonacci of 27
; predefs start
(defn fib [n]
  (if (<= n 1)
    n
    (+ (fib (dec n))
       (fib (- n 2)))))
; predefs end
(fib 27)
