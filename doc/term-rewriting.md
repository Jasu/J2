# Term rewriting for compilation passes

## Matching

Patterns are matched using a LALR parser. The expression tree is iterated
depth-first, with tokens for entering and exiting a node:

```
(defun foo ()
  (if a 1 2))
```

would generate the following token stream:

  1. `enter_defun`
  2. `enter_if`
  3. `enter_sym_val_rd`
  4. `exit_sym_val_rd`
  5. `enter_ld_const`
  6. `exit_ld_const`
  7. `enter_ld_const`
  8. `exit_ld_const`
  9. `exit_if`
  10. `exit_defun` (this *might* be omitted, since the root node has special handling.)

Thus, `(if _ CONST _)` could be matched with the following grammar rule:

```
IfTwo -> enter_if Any enter_ld_const exit_ld_const Any exit_if
```

## Combining rewrites

### Idea #1 - grouping single terms

```
(copy (vec ...)) => (vec ...)
(copy (copy ...)) => (copy ...)
(append (copy ...)) => (copy ...)
...
```

could be defined as

```
VectorAllocatingOp -> vec
VectorAlloc -> VectorCopyingOp(VectorAlloc)
VectorAlloc -> VectorAllocatingOp(Any)

VectorCopyingOp -> copy|append
VectorCopy -> VectorCopyingOp(VectorCopy)
VectorCopy -> VectorCopyingOp(Any)
```

where `VectorCopy` and `VectorAlloc` would track the bottom-most node, discarding
any data between. Ordering of the rules could be handled by priorities, or at least
in the above case by taking the set difference between `AnyOp` and `Vector*Op`

### Idea #2 - Hiding terms from the LALR parser

When replacing e.g. `(size (append $1 $2)) -> (+ (size $1) (size $2))`, the above
technique of manipulating production rules won't work - the parser has already
parsed `$1` and `$2`, and possibly transitioned states because of it. A single
production can only yield a single nonterminal on the stack (obviously, since it
is only a linear stack.) Here two nonterminals would be needed.

The transformation could be made during the top-down descent, e.g. when entering
`append`. However, the LALR parser's state has already been changed to reflect
a single `append` node with two children. To work around this, the lookahead
symbol could be checked right before `enter_append` would be read, replacing it
with a synthetic symbols that build the desired structure. This, along with
the `size` call would be matched with a regular rule, yielding a non-terminal
processable along normal arithmetic expressions.
