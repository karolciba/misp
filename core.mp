; core
; boolean logic
(def! nil ())
(def! false ())
; error on evaluation
; (def! true (fn* () (true)))
; TODO: fix temp definition for true
(def! true 1)
(def! neg
      (fn* (var)
           (cond?
                  (true true)
                  (var false)
           )
      )
)
;(def! if (fn* (var true_clause false_clause)
;              (cond?
;                    (var true_clause)
;                    ((neg var) false_clause)
;              )))

; placeholders for inc and dec
; till numeric support will be raplaced
; by list concatenation
(def! inc (fn* (x) (+ x 1)))

; cons, car and cdr endcoding in lambda
(def! cons (fn* (x y) (fn* (m) (m x y))))
:(def! car (fn* (z) (fn* (p q) (p))))
;(def! cdr (fn* (z) (fn* (p q) (q))))
