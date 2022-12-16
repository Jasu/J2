(defun f (a)
  (let ((res (make-vec 20))
        (idx 0))
    (do-until
      (setp (res[idx]
              ; This let generates a new within the loop.
              (let ((b idx))
                ; Each lambda should have its own copy of b, thus returning a+0, a+1, ...
                (fn () (+ a b))))
            (idx (+ idx 1)))
     (eq idx 20))
    res))
