(def fn
  (fn* (& fdecl)
    (cons 'fn* fdecl)))
(set-macro! (var fn))

; (defn name & fdecl) => (def name (fn & fdecl))
(def defn
  (fn (name & fdecl)
    (list 'def name (cons 'fn fdecl))))
(set-macro! (var defn))

(defn defmacro (name args & body)
  (list 'do
        (list* 'defn name args body)
        (list 'set-macro! (list 'var name))))
(set-macro! (var defmacro))

(defmacro let (bindings & body)
  (list* 'let* bindings body))

(defmacro loop (bindings & body)
  (list* 'loop* bindings body))

(defn identity (x)
  x)

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

(defn reduce
  ((f coll)
   (let (s (seq coll))
     (if s
       (reduce f (first s) (next s))
       (f))))
  ((f init coll)
   (let (s (seq coll))
     (if s
       (recur f (f init (first s)) (next s))
       init))))

(defn concat
  (() nil)
  ((s) s)
  ((s1 s2)
   (let (s (seq s1))
     (if s
       (cons (first s) (concat (next s) s2))
       s2)))
  ((s1 s2 & colls)
   (reduce concat (concat s1 s2) colls)))

(defmacro if-not (test then else)
  `(if ~test ~else ~then))

(defmacro when (test & body)
  `(if ~test
     (do ~@body)))

(defmacro when-not (test & body)
  `(if ~test
     nil
     (do ~@body)))

(defmacro cond (& clauses)
  (when clauses
    (let* (test (first clauses)
           then (if (next clauses)
                    (second clauses)
                    (error "cond requires an even number of forms"))
           more-clauses (next (next clauses)))
      `(if ~test
         ~then
         (cond ~@more-clauses)))))

(defmacro comment (& body))

(defn +
  (() 0)
  ((x) x)
  ((x y) (builtin_binary_add x y))
  ((x y & more)
   (let* (sum1 (builtin_binary_add x y))
     ; todo: reduce
     (reduce + (cons sum1 more)))))

(defn -
  (() 0)
  ((x) (- 0 x))
  ((x y) (builtin_binary_sub x y))
  ((x y & more)
   (let* (diff1 (builtin_binary_sub x y))
     (reduce - (cons diff1 more)))))

(defn *
  (() 1)
  ((x) x)
  ((x y) (builtin_binary_mul x y))
  ((x y & more)
   (let* (prod1 (builtin_binary_mul x y))
     (reduce * (cons prod1 more)))))

(defn /
  (() 1)
  ((x) (builtin_binary_div 1 x))
  ((x y) (builtin_binary_div x y))
  ((x y & more)
   (let* (quot1 (builtin_binary_div x y))
     (reduce / (cons quot1 more)))))

(defn <
  ((x) true)
  ((x y) (builtin_binary_lt x y))
  ((x y & more)
   (if (builtin_binary_lt x y)
     (if (next more)
       (recur y (first more) (next more))
       (builtin_binary_lt y (first more)))
     false)))

(defn <=
  ((x) true)
  ((x y) (builtin_binary_lte x y))
  ((x y & more)
   (if (builtin_binary_lte x y)
     (if (next more)
       (recur y (first more) (next more))
       (builtin_binary_lte y (first more)))
     false)))

(defn reverse (lst)
  (reduce (fn (acc x)
            (cons x acc))
    ()
    lst))

; well... it works, right?
(defn > (& args)
  (apply < (reverse args)))

(defn >= (& args)
  (apply <= (reverse args)))

(defn print
  (())
  ((x) (builtin_unary_print x))
  ((x & more)
  (builtin_unary_print x)
   (loop (xs more)
     (when (seq xs)
       (builtin_unary_print " ") ; TODO : edn vs string: \space
       (builtin_unary_print (first xs))
       (recur (next xs))))))

(defn println (& args)
  (apply print args)
  (builtin_unary_print "\n")) ; TODO : edn vs string: \newline

(defn =
  ((x) true)
  ((x y) (builtin_binary_equal x y))
  ((x y & more)
   (if (builtin_binary_equal x y)
     (if (next more)
       (recur y (first more) (next more))
       (builtin_binary_equal y (first more)))
     false)))

(defn not (x)
  (if x
    false
    true))

(defn not= (x y)
  (not (= x y)))

; todo: = vs == for numbers
(defn zero? (x)
  (if (= x 0)
    true
    (= x 0.0)))

(defn nil? (x)
  (identical? x nil))

(defn comp
  (() identity)
  ((f) f)
  ((f g)
    (fn (& args) (f (apply g args))))
  ((f g & fs)
   (reduce comp (list* f g fs))))

(defn empty (coll)
  (cond (string? coll) ""
        (list? coll) ()))

(defn partial
  ((f) f)
  ((f & args)
   (fn (& more-args)
     (apply f (concat args more-args)))))

(defn gensym
  (() (gensym "G__"))
  ((prefix)
   (symbol (str prefix (next-id)))))

(defmacro and
  (() true)
  ((x) x)
  ((x & more)
   `(let (and# ~x)
      (if and#
        (and ~@more)
        and#))))

(defmacro or
  (() nil)
  ((x) x)
  ((x & more)
   `(let (or# ~x)
      (if or#
        or#
        (or ~@more)))))

(defmacro time (& body)
  `(let (start# (epoch-nanos)
         ret# (do ~@body))
     (println "Elapsed: " (/ (double (- (epoch-nanos) start#)) 1000000) " ms")
     ret#))

(defmacro dotimes (bindings & body)
  (let (iter-var (first bindings)
        n (second bindings))
    `(let (n# (long ~n))
       (loop (~iter-var 0)
         (when (< ~iter-var n#)
           ~@body
           (recur (+ ~iter-var 1)))))))

(defmacro if-let
  ((bindings then)
   `(if-let ~bindings ~then nil))
  ((bindings then else)
   (when (or (not (list? bindings))
             (not= (count bindings) 2))
     (error "if-let requires a list of two elements for bindings"))
   (when-not (symbol? (first bindings))
     (error "if-let requires a symbol as the first element of bindings"))
   (let (form (first bindings)
         tst (second bindings))
     `(let (temp# ~tst)
        (if temp#
          (let (~form temp#)
            ~then)
          ~else)))))

(defmacro when-let (bindings & body)
  (when (or (not (list? bindings))
            (not= (count bindings) 2))
    (error "when-let requires a list of two elements for bindings"))
  (when-not (symbol? (first bindings))
    (error "when-let requires a symbol as the first element of bindings"))
  (let (form (first bindings)
        tst (second bindings))
    `(let (temp# ~tst)
       (when temp#
         (let (~form temp#)
           ~@body)))))

(defmacro -> (x & forms)
  (loop (x x forms forms)
    (if forms
      (let (form (first forms)
            threaded (if (list? form)
                       `(~(first form) ~x ~@(next form))
                       (list form x)))
        (recur threaded (next forms)))
      x)))

(defmacro ->> (x & forms)
  (loop (x x forms forms)
    (if forms
      (let (form (first forms)
            threaded (if (list? form)
                       `(~(first form) ~@(next form) ~x)
                       (list form x)))
        (recur threaded (next forms)))
      x)))

; recursively macroexpands all subforms, not just the top-level form
(defn macroexpand-all (form)
  (let (expanded (macroexpand form))
    (if (list? expanded)
      (map macroexpand-all expanded)
      expanded)))

(defn inc (x)
  (+ x 1))

(defn dec (x)
  (- x 1))
