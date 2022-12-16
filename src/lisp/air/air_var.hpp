#pragma once

#include "lisp/common/id.hpp"
#include "containers/span.hpp"
#include "containers/inline_array_fwd.hpp"

namespace j::lisp::air {
  /// A lexical variable.
  struct air_var final {
    J_BOILERPLATE(air_var, CTOR_NE_ND)

    id name = id::none;
    u32_t index:20 = 0U;

    /// Index of the activation record layout where this variable is stored.
    ///
    /// This is zero for for variables not bound by a closure.
    ///
    /// The following example demonstrates a situation where even single lambdas
    /// will need multiple linked activation records:
    ///
    /// ```
    /// (let ((a 1)  ; Not bound in closure, can stay on stack (act record 0)
    ///       (b 2)) ; Shared by all lambdas created in loop, so one activation
    ///   (do-until  ; on heap suffices. (act record 1)
    ///     (let ((c b)) ; New let for each loop, new activation record needed
    ///                  ; on each iteration (act record 2)
    ///       (foo! (fn (new-b new-c) (set b new-b) (set c new-c))
    ///             (fn () (+ b c))))))
    /// ```
    ///
    /// The numbering continues for closure defined, except for 0 - which means
    /// stack-allocated and does not imply distinctness of activation records.
    /// The number is continuous to make it easy to describe which activation records
    /// each closure needs when there are multiple nested closures binding variables
    /// from various functions.
    u32_t activation_record_index:11 = 0U;

    /// Whether the vatiable is bound by a lambda and must thus be heap-allocted.
    ///
    /// Before activation records are allocated, it cannot be compared to zero,
    /// so vars are explicitly marked as bound.
    u32_t is_bound:1 = 0U;

    J_A(AI,ND) inline air_var(id name, u32_t index, u32_t activation_record_index = 0U) noexcept
      : name(name),
        index(index),
        activation_record_index(activation_record_index)
    { }
  };

}

J_DECLARE_EXTERN_INLINE_ARRAY(j::lisp::air::air_var *);

namespace j::lisp::air {
  using air_vars_t = span<air_var>;
  using air_var_ptrs_t = span<air_var *>;
  using const_air_var_ptrs_t = span<air_var * const>;
  using air_var_defs_t = inline_array<air_var*>;
}
