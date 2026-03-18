(def + builtin_binary_add)

(def print builtin_unary_print)

(def zero? builtin_iszero)

(def fn
  (fn* (args body)
    (list 'fn* args body)))
(set-macro! (var fn))

(def defn
  ;; for now, the body is a single expression: TODO - support '&' for varargs
  (fn (name args body)
    (list 'def name (list 'fn args body))))
(set-macro! (var defn))

(def defmacro
  (fn (name args body)
    (list 'do
          (list 'defn name args body)
          (list 'set-macro! (list 'var name)))))
(set-macro! (var defmacro))

(defn not (x)
  (if x
    false
    true))

(defmacro if-not (cond then else)
  (list 'if cond else then))
