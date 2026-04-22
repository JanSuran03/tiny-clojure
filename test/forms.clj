67
6.9
nil
"Hello, world!"
true
false
(do)
(do 1 2)
(if true 1 2)
(if false 1 2)
(if nil 1 2)
(if 0 1 2)
(if true 1)
(if false 1)
(let [] 1)
(let [a 1] a)
(let [a 1 b 2] a)
(let [a 1 a 2] a)
(let [a 1 b a] b)
(let [add +] (add 1 2))
(let [+ -] (+ 5 2))
((fn [a] (+ a a)) 2)
(let [add (fn [x y]
            (+ x y))]
  (add 1 2))
(let [a 1
      adder (fn [b]
              (+ a b))]
  (adder 2))
(let [make-adder (fn [a]
                   (fn [b] (+ a b)))
      make-multi-adder (fn [a b]
                         (let [adder-a (make-adder a)
                               adder-b (make-adder b)]
                           (fn [c]
                             (adder-b (adder-a c)))))
      adder-2-3 (make-multi-adder 2 3)]
  (adder-2-3 4))
(do (def countdown
      (fn [x]
        (if (zero? x)
          (print "Done.\n")
          (do (print x)
              (print "...\n")
              (countdown (- x 1))))))
    (countdown 3))
((fn [x y]
   (if (zero? x)
     (print "Done.\n")
     (do (print x)
         (print " ")
         (print y)
         (print "...\n")
         (recur (- x 1)
                (+ y 1)))))
 3 3)
(loop [a 5]
  (if (not (zero? a))
    (recur (- a 1))
    1))
(loop [a 5]
  (if (zero? a)
    1
    (recur (- a 1))))
(loop [a 5]
  (if (not (zero? a))
    (recur (- a 1))))
(loop [a 5]
  (if (not (zero? a))
    (let [new-a (- a 1)]
      (recur new-a))
    1))
(loop [a 1 b 1]
  (if (zero? a)
    1
    (recur (if true
             (- a 1)
             (+ a 1))
           (loop [c 0]
             c))))
(let [f (fn [a b a]
          (+ a b a))]
  (f 1 2 3))
(let [a 1
      b 2
      c 3
      f (fn [a] ; a = 4
          (let [b (* 10 a)] ; b = 40
            (+ a b c)))] ; 4 + 40 + 3 = 47
  (f 4))
(loop [x 1 ; 3 iterations, other uses of this variable are shadowed
       sum 0] ; => 6 + 6 + 6 = 18
  (if (> x 3)
    sum
    (let [sum2 (loop [x 1
                      sum 0] ; 1 + 2 + 3 = 6
                 (if (> x 3)
                   sum
                   (recur (inc x) (+ sum x))))]
      (recur (inc x) (+ sum sum2)))))
