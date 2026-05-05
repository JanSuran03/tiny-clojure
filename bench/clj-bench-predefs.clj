(defmacro bench [& body]
  `(let [start# (System/nanoTime)
                ret# (do ~@body)]
     (/ (double (- (System/nanoTime) start#)) 1000000))) ; return the time in ms