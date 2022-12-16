#pragma once

#include "containers/span.hpp"

namespace j::lisp::air {
  struct air_var;
}

namespace j::lisp::air::exprs {
  struct expr;

  struct vars_init final {
    air_var * first = nullptr;
    u32_t num_vars = 0U;

    J_A(AI) inline vars_init(span<air_var> from) noexcept
    : first(from.begin()),
      num_vars(from.size())
    { }

    J_A(AI,ND) inline explicit vars_init(u32_t size) noexcept
    : num_vars(size)
    { }
  };

  J_A(AI,NODISC,ND,HIDDEN) inline vars_init allocate_vars(u32_t size) noexcept {
    return vars_init(size);
  }

  struct inputs_init final {
    u8_t num_allocated = 0U;
    u8_t num_initialized = 0U;
    expr * const * first = nullptr;

    J_A(AI,ND) inline inputs_init(span<expr * const> from) noexcept
    : num_allocated(from.size()),
      num_initialized(num_allocated),
      first(from.begin())
    { }

    J_A(AI,ND) inline inputs_init(span<expr *> from) noexcept
    : num_allocated(from.size()),
      num_initialized(num_allocated),
      first(from.begin())
    { }

    template<u32_t N>
    J_A(AI) inline inputs_init(expr * const (&inputs)[N]) noexcept
    : num_allocated(N),
      num_initialized(N),
      first(inputs)
    { }

    J_A(AI,ND) inline inputs_init(u32_t num_allocated, u32_t num_initialized) noexcept
    : num_allocated(num_allocated),
      num_initialized(num_initialized)
    { }
  };


  J_A(AI,NODISC) inline inputs_init reserve_inputs(u32_t size) noexcept {
    return inputs_init(size, 0U);
  }

  J_A(AI,NODISC) inline inputs_init allocate_inputs(u32_t size) noexcept {
    return inputs_init(size, size);
  }

  using alloc_inputs = inputs_init;
  using alloc_vars = vars_init;
}
