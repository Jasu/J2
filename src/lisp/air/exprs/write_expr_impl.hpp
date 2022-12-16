#pragma once

#include "hzd/mem.hpp"
#include "lisp/air/exprs/write_utils.hpp"
#include "lisp/air/exprs/expr.hpp"
#include "lisp/air/air_var.hpp"

namespace j::lisp::air::exprs {
  namespace {
    [[maybe_unused]] J_A(NI) inline void write_dynamic_inputs(input * J_NOT_NULL to, val_spec spec,
                                                              expr * J_NOT_NULL const parent, u8_t index,
                                                              inputs_init init) noexcept {
      if (init.first) {
        for (expr *const *it = init.first, *const *end = it + init.num_initialized;
             it != end; ++it, ++to, ++index) {
          ::new (to) input{*it, spec};
          if (to->expr) {
            to->expr->parent = parent;
            to->expr->parent_pos = index;
          }
        }
      } else {
        for (u32_t i = 0U; i < init.num_allocated; ++i) {
          (to++)->type = spec;
        }
      }
    }

    [[maybe_unused]] inline void write_air_var_defs(void * J_NOT_NULL to, vars_init in) noexcept {
      auto vars = ::new (to) air_var_defs_t(in.num_vars);
      if (air_var * it = in.first) {
        for (air_var *e = it + in.num_vars, **wr = vars->begin(); it != e; ++it, ++wr) {
          *wr = it;
        }
      }
    }
  }
}
