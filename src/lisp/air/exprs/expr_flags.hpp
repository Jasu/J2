#pragma once

#include "util/flags.hpp"

namespace j::lisp::air::exprs {
  enum class expr_flag : u8_t {
    /// Has observable side-effects, i.e. creates an ordering barrier.
    has_side_effects,
    /// May perform a non-local exit, i.e. throw, OOM, longjump, abort, or unwind stack by some other means.
    may_nlx,
    /// Automatically inserted check or OOM condition, that is moveable among other automatically inserted exprs.
    movable_nlx,
    /// Reads global state. Writes form a barrier for this.
    reads_memory,
    /// Writes global state. Reads form a barrier for this.
    writes_memory,
    /// Reads lexical variables.
    reads_lex,
    /// Writes lexical variables.
    writes_lex,
    /// F(X, Y) = F(Y, X)
    commutative,

    /// F(X, Y, Z) = F(X, Z, Y)
    ///
    /// The first element cannot be reordered. Subtraction and division with more
    /// than two elements have this property.
    commutative_tail,

    /// F(X, Y, Z) = F(X, F(Y, Z)) = F(F(X, Y), Z)
    ///
    /// When converted to binary expressions, any ordered representation of the
    /// full binary expression tree is valid.
    ///
    /// For example concatenation is associative (but not commutative).
    associative,
    /// F(X, X) = X
    ///
    /// Operands may ne deduplicated. E.g. bitwise `or` and `and` have this property.
    idempotent,

    /// F(F(X)) = F(X)
    ///
    /// Iterated application of the function makes no change.
    /// E.g. `abs` and type casts have this property.
    composition_idempotent,

    /// Function is its own inverse: F(F(X)) = X
    ///
    /// For example, negation and bitwise not are involutory.
    involutory,

    /// F(X) = X, when used as unary version.
    unary_identity,

    /// Expression data contains a constant C such that for all X: F(X, C) = X and F(C, X) = X
    has_identity_element,

    /// Falsy values are identity elements.
    identity_element_falsy,

    /// Expression data contains a constant C such that for all X: F(X, C) = X
    has_identity_element_tail,

    /// Expression data contains a constant C such that for all X: F(X, C) = C and F(C, X) = C
    has_absorbing_element,

    /// Evaluating the expression with no arguments yields a constant.
    ///
    /// E.g. (+) evaluates to `0` and (*) evaluates to `1`.
    is_nullary_constant,

    /// Evaluating the expression with a single argument yields a constant.
    ///
    /// E.g. (eq a) evaluates `true` and (neq a) evaluates to `false`.
    is_unary_constant,

    /// The value of the expression is the value of its last subexpression.
    ///
    /// E.g. (eq a) evaluates `true` and (neq a) evaluates to `false`.
    prognish,

    /// All arguments are not necessarily evaluated.
    is_special,

    /// The expr defines vars, i.e. is a let or a function body.
    has_vars,
  };

  using expr_flags = util::flags<expr_flag, u32_t>;
  J_FLAG_OPERATORS(expr_flag, u32_t)

  constexpr inline expr_flags expr_flags_non_elidable_v{expr_flag::may_nlx, expr_flag::has_side_effects, expr_flag::writes_memory, expr_flag::writes_lex, expr_flag::is_special, expr_flag::has_vars};
}
