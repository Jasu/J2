#pragma once

#include "lisp/air/air_var.hpp"
#include "lisp/air/exprs/expr.hpp"
#include "lisp/air/exprs/expr_counts.hpp"
#include "lisp/air/functions/activation_record.hpp"

namespace j::lisp::air::inline functions {
  struct air_closure;

  /// A top-level (non-lambda) function.
  struct air_function final {
    air_function() noexcept;
    J_A(AI,ND) inline explicit air_function(exprs::expr * J_NOT_NULL body) noexcept
      : body(body) {
    }

    /// The main body of the function. Lambdas are defined separately.
    exprs::expr * body = nullptr;
    /// For marking expressions visited in `air_pass`es.
    ///
    /// Represents the currently expected state of the flag in an `expr`.
    /// By flipping this after marking all `expr`s as visited, the graph
    /// only needs to be visited once.
    ///
    /// Note that creating new `expr`s and not initializing its flag to the
    /// expected value is a bountiful source of bugs.
    bool visit_flag_value = false;
    /// Number of lexical variables, including those defined in child closures.
    u32_t num_lex_vars = 0U;
    /// Number of closures defined, including those nested in other closures.
    u32_t num_closures = 0U;
    /// Maximum scope depth of a closure.
    u32_t max_closure_depth = 0U;
    /// Head of a linked list of closures
    air_closure * first_closure = nullptr;
    /// Tail of a linked list of closures.
    air_closure * last_closure = nullptr;

    /// Different activation records used by closures.
    activation_records activation_records;
    span<air_var*> lex_vars;

    exprs::expr_counts counts;
  };
}
