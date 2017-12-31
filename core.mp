; core
; boolean logic
; empty list evaluates to false/nil
(def! nil ())
(def! false ())
; true if everything else, it lambda returning itself
(def! true (fn* () (true)))
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

; cons, car and cdr encoding in lambda
; Church encoding see Wikipedia:
; https://en.wikipedia.org/wiki/Cons#Functional_Implementation
(def! lcons
      (fn* (x y)
           (fn* (m)
                (m x y)
           )
      )
)

(def! lcar
      (fn* (z)
           (z (fn* (p q) p))
      )
)

(def! lcdr
      (fn* (z)
           (z (fn* (p q) q))
      )
)

(def! inc
      (fn* (x)
           (cons true x)
      )
)

(def! dec
      (fn* (x)
           (cdr x)
      )
)
