(def! nil ())
(def! false ())
(def! true (fn* () nil))
(def! neg (fn* (var) (cond? (var ()) (1 true))))

(def! inc (fn* (x) (+ x 1)))
(def! cons (fn* (x y) (fn* (m) (m x y))))
(def! car (fn* (z) (fn* (p q) (p))))
