# IR notes

## Entering SSA form

  - Since no `goto` or the like (e.g. `tagbody`) is allowed, SSA form can be determined in a
    single pass from AIR.

## Register allocation

  - Linear scan in SSA form (Wimmer et al.) is used for register allocation.
  - First, the compilation target is queried for number of registers available.
  - Then, fit is made for that number of registers - all yet virtual.
  - Then, machine registers are assigned - first for function arguments, then
    for the return value.
  - Machine registers are not allocated directly, ot be able to ensure that
    arguments and the return value end up in the correct registers.

## AIR vs. CIR

### Pro AIR

  - Context of the expression is known
    - E.g. loop variables are explicit
  - Alias processing is possible (in theory)


## Closures

Closures close over variables, not over values. This makes the following code a bit difficult to implement:

```
(defun foo ()
  (let ((a 0))
    (vec
      (let ((b 0))
        (vec
          (fn () (+ a b))
          (fn (new-a new-b) (set a new-a) (set b new-b))))
      (let ((b 0))
        (vec
          (fn () (+ a b))
          (fn (new-a new-b) (set a new-a) (set b new-b)))))))
```

Basically, the functions reference the same `a` but different `b` - but writes to `a`
by either has to be visible in both of them. This implies that the closure context cannot
be a flat vector - it has to be a linked list, representing the variables on stack
that were bound - i.e. a static chain:

```
(let* ((static-chain-a (vec nil 0))
       (static-chain-b1 (vec static-chain-a 0))
       (static-chain-b2 (vec static-chain-a 0))
       (getter (fn (static-chain) (+ (get (get static-chain 0) 1) (get static-chain 1))))
       (setter (fn (static-chain a b) (set (get static-chain 0) 1 a) (set static-chain 1 b))))
  (vec
    (vec
      (with-static-chain getter static-chain-b1)
      (with-static-chain setter static-chain-b1))
    (vec
      (with-static-chain getter static-chain-b2)
      (with-static-chain setter static-chain-b2))))
```

[^cooper]: Keith D. Cooper, Timothy J. Harvey, and Ken Kennedy: **A Simple, Fast Dominance Algorithm** (2006)
