(def + builtin_binary_add)
(def - builtin_binary_sub)
(def * builtin_binary_mul)
(def / builtin_binary_div)
(def print builtin_unary_print)
(def zero? builtin_iszero)

(def fn
  (fn* (args & body)
    (cons 'fn* (cons args body))))
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
                    (error "cond: missing else clause"))
           more-clauses (next (next clauses)))
      (list 'if test then (cons 'cond more-clauses)))))

(defmacro comment (& body))
