#include "expr_info.hpp"
#include "lisp/air/exprs/expr.hpp"
#include "lisp/air/air_var.hpp"

namespace j::lisp::air::exprs {
  [[nodiscard]] u32_t get_expr_size(const expr * J_NOT_NULL e) noexcept {
    auto & i = expr_sizes_v[(u8_t)e->type];
    u32_t result = i.static_size;
    J_ASSERT(result);
    if (i.has_dynamic_inputs) {
      J_ASSERT(e->num_inputs >= i.num_static_inputs);
      result += (e->max_inputs - i.num_static_inputs) * sizeof(input);
    } else {
      J_ASSERT(e->num_inputs == i.num_static_inputs);
    }
    if (i.has_var_defs) {
      result += air_var_defs_t::get_byte_size(((const air_var_defs_t*)e->static_begin())->size());
    }
    return align_up(result, 8U);
  }
}
