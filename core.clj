(def fn
  (fn* (& fdecl)
    (cons 'fn* fdecl)))
(set-macro! (var fn))

(def defn
  (fn (name args & body)
    (list 'def name (cons 'fn (cons args body)))))
(set-macro! (var defn))

(defn defmacro (name args & body)
  (list 'do
        (cons 'defn (cons name (cons args body)))
        (list 'set-macro! (list 'var name))))
(set-macro! (var defmacro))

(defmacro let (bindings & body)
  (cons 'let* (cons bindings body)))

(defn identity (x)
  x)

(defn not (x)
  (if x
    false
    true))

(defmacro if-not (test then else)
  (list 'if test else then))

(defn map (f lst)
  (if (seq lst)
    (cons (f (first lst)) (map f (next lst)))
    ()))

(defn filter (f lst)
  (if (seq lst)
    (if (f (first lst))
      (cons (first lst) (filter f (next lst)))
      (filter f (next lst)))
    ()))

(defn second (lst)
  (first (next lst)))

(defmacro when (test & body)
  (list 'if test (cons 'do body)))

(defmacro cond (& clauses)
  (if clauses
    (let* (test (first clauses)
           then (if (next clauses)
                    (second clauses)
                    (error "cond requires an even number of forms"))
           more-clauses (next (next clauses)))
      (list 'if test then (cons 'cond more-clauses)))))

(defmacro comment (& body))

(defn +
  (() 0)
  ((x) x)
  ((x y) (builtin_binary_add x y))
  ((x y & more)
   (let* (sum1 (builtin_binary_add x y))
     ; todo: reduce
     (apply + (cons sum1 more)))))

(defn -
  (() 0)
  ((x) (- 0 x))
  ((x y) (builtin_binary_sub x y))
  ((x y & more)
   (let* (diff1 (builtin_binary_sub x y))
     (apply - (cons diff1 more)))))

(defn *
  (() 1)
  ((x) x)
  ((x y) (builtin_binary_mul x y))
  ((x y & more)
   (let* (prod1 (builtin_binary_mul x y))
     (apply * (cons prod1 more)))))

(defn /
  (() 1)
  ((x) (builtin_binary_div 1 x))
  ((x y) (builtin_binary_div x y))
  ((x y & more)
   (let* (quot1 (builtin_binary_div x y))
     (apply / (cons quot1 more)))))

; todo
(def print builtin_unary_print)

(defn =
  ((x) true)
  ((x y) (builtin_binary_equal x y))
  ((x y & more)
   (if (builtin_binary_equal x y)
     (if (next more)
       (recur y (first more) (next more))
       (builtin_binary_equal y (first more)))
     false)))

; todo: = vs == for numbers
(defn zero? (x)
  (if (= x 0)
    true
    (= x 0.0)))
