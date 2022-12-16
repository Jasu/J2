#pragma once

#include "hzd/iterators.hpp"
#include "lisp/air/air_var.hpp"

namespace j::lisp::air::inline functions {
  struct closure_binding final {
    J_ALWAYS_INLINE closure_binding(air_var * J_NOT_NULL var, closure_binding * next) noexcept
      : var(var), next(next)
    {
    }

    air_var * var = nullptr;
    closure_binding * next = nullptr;
  };

  using closure_binding_iterator = linked_list_iterator<closure_binding, &closure_binding::var>;
  using const_closure_binding_iterator = linked_list_iterator<const closure_binding, &closure_binding::var>;
}
