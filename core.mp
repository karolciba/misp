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
(def! if
      (fn* (var true_clause false_clause)
           (cond?
                 (true false_clause)
                 (var true_clause)
           )
      )
)

; placeholders for inc and dec
; till numeric support will be raplaced
; by list concatenation
(def! inc (fn* (x) (+ x 1)))

; cons, car and cdr encoding in lambda
; Church encoding see Wikipedia:
; https://en.wikipedia.org/wiki/Cons#Functional_Implementation
(def! cons
      (fn* (x y)
           (fn* (m)
                (m x y)
           )
      )
)

(def! car
      (fn* (z)
           (z (fn* (p q) p))
      )
)

(def! cdr
      (fn* (z)
           (z (fn* (p q) q))
      )
)

(def! z (cons 1 2))
(car z)
(cdr z)
(def! y (cons 3 z))
(car y)
(cdr y)
