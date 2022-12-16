# Term rewriting notes

General notes about term rewriting, not necessarily relating to the code.

## General definitions

A term rewriting system consists of 𝚺-terms - including functions of specified
airty, and of constant values (which can be thought of 0-arity functions), and
of variables in X. The 𝚺-term `T(𝚺,X)` set contains all possible terms with the functions
in sigma, and with the countably infinite number of variables in X.

**Substitution** does **not** mean replacing a term with another, but rather
replacing a finite set of **variables** with terms from 𝚺. Substitution is denoted
by σ, mapping a variables from X to T(𝚺,X). Substitution is defined for whole `X`,
but it maps most variables to themselves, only substituting a finite number of them.
The input variables altered is called its domain (Dom) and the terms they map to is
called its range.

## Notes

### How do term rewriting systems relate to equational etc. algebras?

In term rewriting systems, rules are directional, not equational - much like
production rules in formal languages. Converting an algebra to a TRS would
make it non-terminating, since a rule and its inverse could be applied ad infinitum.

Symmetrical rules contain no information on which side of the rule is preferrable.
Converting an algebra to a TRS while selecting directions for the rules is what
brings the normal form to existence. When simplifying equations, it is usually "obvious"
which form is simpler. However, the "obvious" heuristics are likely decided rather
arbitrarily, stemming from e.g. to have a physically smaller formula, or to make answers
in math tests more easily checkable, or they might stem from some aesthetic preference
that is not easily compilable to machine code.

The same algebra might have multiple well-defined normal forms, e.g. boolean
algebras have conjunctive and disjunctive normal forms. Sometimes it might be useful
to convert to some other normal form, perform a transformation, and then convert
back to the first normal form. Some effective solutions to math problems do just that,
or more often first transform the problem to some other domain, then convert to a
normal form, and then convert back (e.g. solving differential equations by taking a
scenic route through Fourier or Laplace domain.)

## Non-terminating TRSs

There are two cases that may cause non-termination - infinite loops among the set of rules,
and infinite expansion of the expression:

  - `F(a, b) -> F(b, a)` leads to an infinite loop. Thus, associativity cannot be implemented
    in direct form.
  - `F(a) -> F(a) + F(a)` leads to infinite expansion. Note that this implies that non-termination
    is not always indicative of cycles in the rewrite rules.

For a TRS operating on a finite alphabet, there can be no other cases, since if the number of
symbols is finite, setting a bound on the size of the expression (i.e. preventing infinite expansion)
would also set a bound on possible inputs. A non-terminating system would necessarily have to visit
the same state multiple times - i.e. be stuck in a loop.

## Glossary

Completion
: Adding rules to the TRS, e.g. to make it confluent.

Confluence
: If order in which rewrite rules are applied does not affect the final derivation,
: the rewrite system is confluent.

Contraction
: Applying a redex

Finitely branching
: Each node has a finite number of successors (i.e. number of rewrite rules per term
: is finite)

Globally finite
: If each element has a finite number of successors (i.e. the number of rewrite rules
: overall is finite, given that the alphabet is finite)
: Note: any acyclic relation that is globally finite terminates.
: See König's Lemma

Ground term
: Term with no variables, i.e. a terminal node.

Irreducible
: Term to which no rewrite rules apply. I.e. it is in normal form.

König's Lemma
: A finitely branching tree is infinite **if and only if** it contains
: an infinite path.
: Note: Applies to trees, not graphs.

Many-sorted
: Partitioning of a TRS based on applicability of rules. E.g. `(* "Hello" "World)`
: would not be tried at all, since `*` would be partitioned to a separate set of rules

Order-sorted
: Nesting of a TRS based on applicability of rules. E.g. `(+ 17 0.5)` would
: work because floating-point numbers and integers are both numbers, but
: `(gcd 0.5)` would not be applied, since floats are not integers.
: Implementing order-sorting throws lots of proofs out of the window and is
: thus rather hard to implement.

Measure function
: Function which maps elements from the TRS to some other reduction system,
: e.g. (Nat, >) in a monotonic way. This is used to prove termination.
: If e.g. for `(X, ->)` exists a mapping `f: X -> Nat` such that
: `x1 -> x2 ⇒ f(x1) > f(x2)`, the system terminates.

Normal form
: Form to which no rewrite rules may be applied, i.e. form in whicn terms are are
: irreducible. Note that the term itself does not imply uniqueness.

Sentential form
: Expression derivable from a formal language, starting with the root nonterminal.
: The word is usually used in contexts where the derivation order matters, e.g.
: "right-sentential form", meaning expressions derivable using the right-most
: derivation.

Strict order `<`
: Order that is **irreflexive**, i.e. `!(a < b)`.
: Note that strictness is separate from partial, i.e. generally it does not mean
: that all pairs are comparable - that feature is called **totality**.

Total order
: Order (either strict `<` or non-strict `<=`, `=>` etc.) that can be applied to
: all pairs in its domain.

Total on ground temrs
: Ordering which gives a total and linear (strict) order for all non-terminals (constants).

Unique normal form x↓

Rank
: Arity, or the highest arity of a rule.

Redex
: Reducible expression, i.e. the left-hand-side of a rewrite rule.
: Applying a redex is called contraction.

Reducible
: Term is reducible if any rule can be applied to it, i.e. it is not in normal form.

Term
: An expression built from the functions, constants (terminals), and variables of
: the system. E.g. `F(x, F(y, 0))`, `0`, and `y` - so `term`s are not only limited
: to e.g. terminal symbols, symbols, without variables, etc.

Termination / Terminating relation
: Property of a TRS so that for all expressions, a normal form is reached after
: a finite number of steps.

Well-founded relation
: Synonym for terminating relation.

Well-partial order **(wpo)**
: For every infinite sequence of elements, there exists a pair of elements where
: `ai <= aj, i < j` - i.e. every infinite sequence has a lower bound.


Word problem
: Whether a term can be converted to another one with the rules given. Note that this
: might be a bit hard - in general, it is undecidable (for TRSes, and otherwise). This
: is solvable for TRSes where all terms lead to some unique normal form (i.e. systems
: that are confluent and terminating). In these cases, the compared terms may simply
: be converted to their normal forms and compared as strings.

## Abstract reduction systems

A set `A` and a single operation, called reduction, `->` that can be any function that
maps from `A` to `A`. `->` does not need to apply to all values in `A`.

### Glossary

`i + 1`-fold composition
: Repeatedly applying the reduction to its own result, `i + 1` times.

Closure
: Foo closure of X is the smallest set containing X and having property `Foo`.

Closed under substitutions
: Rewrite relation is closed under substitution if s -> t implies σ(s) -> σ(t) for
: all substitutions and rewrite rules. I.e. the relation does not depend on specific
: subterms.

Closed under 𝚺-operations
: Rewrite relation is closed under sigma-operations if `sn -> tn...` implies
: `f(sn...) -> f(tn...)` for all functions and terms - i.e. wrapping, replacing,
: or unwrapping `f` does not change the result.

Transitive closure +⃯
: Union of all `i + 1`-fold compositions of `->`, for `i > 0` (this restriction excludes
: identity). I.e. if `->` contains `a -> b`,  `b -> c`, and `c -> d`, the transitive closure
: will contain all of them, and `a -> c`, `a -> d`, and `b -> d`.
: Note that even if identity is ruled out in the definition, transitive closure of e.g.
: `a -> b`, `b -> a` will contain identity. I.e. transitive closure will contain an identity
: reduction for all elements that are part of a cycle.

Reflexive closure
: The closure operator itself, with identity included.

Reflexive transitive closure *⃯
: Transitive closure with identity included.

Symmetric closure
: Union of the closure operator and its inverse.

Transitive symmetric closure
: Transitive closure of the symmetric closure. I.e. converts the relation into its reachability
: graph (sans identity).

Reflexive ransitive symmetric closure
: Essentially converts the relation to its connected components.

Successor
: `y` is a successor of `x`, if it is reducible from `x` in the transitive closure of `->`.
: i.e. `y` need not be a direct successor of `x`.

Joinable `x↓y`
: `x` and `y` are joinable if there exists a (possibly non-unique) `z` for which
: x *⃯ z and y *⃯ z. Note that the closure is also reflexive, so terms are joinable with
: themselves. Note also that `z` does not need to be a normal form.

Church-Rosser `x *͍ y ⇒ x↓y` - **Equivalent to confluence**
: Relation `→` is Church-Rosser iff `x *͍ y` implies `x↓y`.
: Clearly `x↓y` also implies `x *͍ y`, so it is equivalence rather than implication)
: Reflexice transitive symmetric closure converts the graph to an undirected graph, and then
: converts it to its reachability graph. So, all elements in parts of the graphs connected
: by edges, wrong way or not, must have a common successor. Thus, the graph must be a set of
: disjoint semilattices, or their closures (potentially having loops).

Confluence `a *⃯ b ∧ a *⃯ c ⇒ b ↓ c`. - **Equivalent to Church-Rosser**
: If `a *⃯ b` and `a *⃯ c`, then `b ↓ c`.
: I.e. if paths diverge in a graph, they must converge later. I.e. all elements of elements
: reachable from a node must have a meet node. Thus, the graph must consist of disjoint
: semilattices, or their closures (potentially having loops).
: Note that confluence seems weaker than Church-Rosser, since the reduction is not symmetric.
: Only nodes reachable via directed graphs must join later, rather than all nodes in a component.
: Yet, this is equivalent to Church-Rosser.

Diamond property `a → b ∧ a → c ⇒ ∃d: b → d ∧ c → d`
: Stronger than confluence (when generalized to the whole system) - i.e. all branches
: must immediately join back.

Local confluence `a → b ∧ a → c ⇒ b↓c` - **Confluent when terminating**
: Weaker variant of confluence, considering only direct children of branching nodes.
: For terminating systems, it is equivalent to confluence (**Newman's Lemma**)

Semi-confluent `a → b ∧ a *⃯ c ⇒ b ↓ c`. - **Equivalent to confluence**
: If `a → b` and `a *⃯ c`, then `b ↓ c`.
: I.e. all direct successors of `a` must eventually meet with all successors of `a`.
: Proof of equivalence:
:   1. I'm standing at node `a`, ready to break free. `c` is away on family business,
:      but still reachable. This should give me a fair bit of lead on my way to freedom.
:      To avoid `c`, I'm going to enter a descendant node away from him.
:   2. Now I'm standing at `b`, a direct descendant of `a`. Luckily, `c` seems not to be
:      reachable anymore, though the meet point `b ↓ c` still is. Not for long though - I
:      think I only need one step to escape once and for all - after all that's what the
:      theorem promised me.
:   3. So I'll enter a direct descendant, say, `b'` - to get away from the meet point `b ↓ c`.
:      Finally, `c` will only be a distant memory.
:   4. Now I'm standing at `b'` - even `b ↓ c` is not reachable! Though I see a meet point
:      of `b'` and `b ↓ c`, `b' ↓ b ↓ c` at the horizon. I guess that's to be expected,
:      since it was reachable from `b`. Maybe a step further.
:      ...
:   9192. I'm standing at node `b'''''''''''''''''''''''''''''''''''` - if `c` truly loved
:      me, he would have come looking for me. He never did, yet the inevitability of
:      meeting him seems to follow me everywhere I go. My food and water supplies are running
:      low and I don't have much time - should I use my last strength to confront my fears,
:      or die alone here? I only now realize that this is the only true choice I have beem
:      given in life. Alas, life on this godforsaken semilattice hellscape never prepared me
:      for such.
:      ...
:   123211. I'm standing at node `b''''''''''''''''''''''''''''''''''''''''''''''''''''''`

Strong confluence `a → b ∧ a → c ⇒ a ∃d: b ⥱ d ∧ c *⃯ d`.
: Note that the symmetry in left side "mirrors" to the right side,
: actually causing this requirement:
: `a → b ∧ a → c ⇒ a ∃d: c ⥱ d1 ∧ b *⃯ d1`.
: `a → b ∧ a → c ⇒ a ∃d: b ⥱ d2 ∧ c *⃯ d2`.

Substitution σ (small sigma)
: Mapping from variable in `X` (of `T(𝚺,X)`) to `T(𝚺,X)`.
: There must be a finite number of `x` that the substitution replaces with
: something else. This input set is called **domain** (Dom). The output set (for
: the actually substituted variables) is called **range**. Substitution is said
: to **instantiate** the input variable it is replacing.
: Substitution operates only on a single variable, not a signature list etc.
: Often σ is applied to function terms, meaning that it is mapped to all function
: arguments. This is called **extension**, but the distinction is often omitted as trivial.

Convergent
: Both confluent and terminating

Newman's Lemma
: Local confluence is equivalent to confluence in terminating systems.

Normalizing (adj.)
: Every element has a (potentially non-unique) normal form.
: The important word here is the "a". In addition to the normal form, there may
: exist cycles or (if infinite graphs are permitted) infinite linear paths, meaning
: that normalizing does not imply terminating.


Nonterminating and confluent with a unique normal form, with cycle makiiiing it nonterminating:
```
A<---->B--->C
```

Nonterminating, with a unique normal form (but infinite graph):
```
  Y -----> Z ------ W-- - - -
  |        |        |
  +<-------+<-------+<- - - -
  |
  V
  N
```

### Properties of confluence

 - Confluence: `a *⃯ b ∧ a *⃯ c ⇒ b↓c`
 - Church-Rosser: (equivalent to confluence) `x *͍ y ⟺ x↓y`

For terminatable, confluent TRSs Church-Rosser implies that every term reachable
via any edge, disregarding directedness, has to end up at the same normal form.

Thus, the graph can be divided to connected parts, which can be directly applied.

If the TRS is not terminable, each term still has **at most** one normal form - i.e.
in cases where it terminates, it terminates in a proper way.

### Termination

Obvious identities:

  - Termination for TRS in general is undecidable
  - Transitive closure +⃯ of → terminates *iff* → terminates.

#### Embedding

Since terminating TRSs form a partial order, where lower levels are closer to
normal form, embedding the TRS to some other known terminating system works.
This sound like a lot of work, but the other system may be (Nat,>), i.e. reduction
system where larger natural numbers are always mapped to smaller ones. So to prove
termination of system (X,->), a **monotone** mapping from (**measure function**)
`f` from X to Nat has to be found. Monotonicity here means that the ordering of
successors is preserved.

If e.g. for `(X, ->)` exists a mapping `f: X -> Nat` such that
`x1 -> x2 ⇒ f(x1) > f(x2)`, the system terminates.

    A finitely branching reduction terminates **iff** there exists a
    monotone embedding to Nat

Finding such embeddings is not easy though.

#### Lexicographic orders

Lexicographic product: (x,y) > (x',y') ⟺ x > x' ∨ (x = x' ∧ y > y')
I.e. just what it sounds like.

    Lexicographic product of two terminating reduction systems is
    terminating.

Thus, terms of two systems can be concatenated, and another system be "given
priority" without sacrificing termination.

This is also rather useful for actual strings of finite length - but also
for dynamic length, if length is compared first, allowing only transforms
to shorter or equal-length strings (equal length must be lexicographically
smaller).

**Note:** Lexicographical orders for partial orders (reflexive reductions,
i.e. those including identity) work, but not directly:
`(ReflexiveClosure A) >= (ReflexiveClosure B)` is different from
`(ReflexiveClosure A > B)` - This is obvious when considering e.g lexicographical
product of natural numbers:

```
    (n,m) >= (n',m') <=> n >= n' V (n == n' ^ m >= m')
<=> (n,m) >= (n',m') <=> n > n' V n == n' V (n == n' ^ m >= m')
<=> (n,m) >= (n',m') <=> n > n' V n == n'
```

The problem was that the equality condition got duplicated. So partial orders
should be combined with

```
(n,m) >= (n',m') <=> n > n' V (n == n' ^ m >= m')
```

I.e. the most significant term must still be handled as a strict order.

#### Multiset orderings

Complex termination systems can be embedded to multisets, i.e. sets where each member
is associated with a count. Embedding to a multiset gives freedom to apply rules
that make the terms seemingly more complex, or that make changes that don't obviously
simplify the term. The multiset must still get strictly smaller on each step, but
the measure of strictly smaller is rather interesting.

First off, the **set elements are ordered** in addition to the counts. Larger set
elements trump larger counts - so multiset ordering works kind of like "dynamic"
lexical ordering.

The criteria for multiset order M < N is:


 - M != N
 - For each element `x` that `M` has more of than `N`, `N` has an element `y`
   such as `y > x` (i.e. set values are ordered - more values of smaller-numbered
   set items are permitted) and `N` has strictly more of those.
 - So, `M` is obtainable from `N` by removing an element and replacing it with
   any number of smaller elements. **Note** the sum of the values in the multiset
   can grow - the bound on the largest number contained in the multiset approaches
   zero, so it will eventually limit the contents.

```
M(5,5)   > M(5,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4)
M(6)     > M(5,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4)
```

Basically, the value can be sorted in a descending order and compared from left to right.
This forces larger numbers to be removed to add smaller numbers - eventually, the larger
numbers will run out. no matter how many copies of smaller numbers are added.

### Non-termination

For non-terminating TRSs, breadth-first search can be used to find normal forms.
Depth-first search might never terminate, even for terms that could be normalized.
Of course, running a non-terminating algorithms is kinda sus.


## Confluence

 - Confluence: `a *⃯ b ∧ a *⃯ c ⇒ b↓c`
 - Church-Rosser: (equivalent to confluence) `x *͍ y ⟺ x↓y`

In general case, hard to prove - enumerating paths over a graph for all elements is
not super fast. Luckily, for terminating systems that is not necessary.

 - Local confluence: `a → b ∧ a → c ⇒ b↓c`

Local confluence is only concerned with immediate descendants of branching paths, but
for terminating systems, it is equivalent to confluence. (**Newman's Lemma**)

## Reduction/rewrite orders

Rewrite order is a strict ordering on all 𝚺-terms terms of the rewrite system (i.e.
not on rewrite rules, but indibidual terms). A rewrite order must satisfy:

  - `a > b` => `f(s1...s_i-1, a, s_i+1...s_n) > f(t1...t_i-1, a, t_i+1...t_n)`
  - be closed under substitution (variable replacement).
  - If the order is **well-founded**, it is additionally a **reduction order**.

A TRS translates **iff** there exists a **reduction order** over its rules.

### Simplification orders (like Knuth-Bendix)

A reduction order `>` is also a **simplification order** if for all terms `t` and
their subterm positions `Pos` iff `t > t[Pos]`.

This does not mean that the system cannot be recursive - numbering is applied to
complete terms, not individual symbols, thus they can account for the depth of
the term. In fact, the orderings have to be constructed recursively.

  - Polynomial ordering
  - Lexicographical/multiset recursive path ordering
    - Method for ordering can be decided on a function-by-function basis.
  - Knuth-Bendix ordering (accepts weights for terms)
