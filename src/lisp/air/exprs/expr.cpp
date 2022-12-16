#include "lisp/air/exprs/expr.hpp"
#include "lisp/air/exprs/expr_info.hpp"
#include "lisp/common/metadata_init.hpp"
#include "lisp/air/air_var.hpp"

#include "containers/inline_array.hpp"
J_DEFINE_EXTERN_INLINE_ARRAY(j::lisp::air::air_var *);

namespace j::lisp::air::exprs {
  [[nodiscard]] metadata_view expr::metadata() const noexcept {
    return {reinterpret_cast<const struct metadata*>(has_metadata ? add_bytes(this, get_expr_size(this)) : nullptr)};
  }

  void write_expr_metadata(expr * J_NOT_NULL to, const metadata_init & init) noexcept {
    to->has_metadata = true;
    init.write(add_bytes<char*>(to, get_expr_size(to)));
  }
  [[nodiscard]] const lisp_imm & expr::get_const() const noexcept {
    J_ASSUME(type == expr_type::ld_const);
    return *reinterpret_cast<const lisp_imm*>(static_begin());
  }

  [[nodiscard]] u32_t expr::depth() const noexcept {
    u32_t result = 0U;
    for (const expr * e = this; e->parent; e = e->parent) {
      ++result;
    }
    return result;
  }

  J_A(RNN) expr * expr::set_input(u8_t index, expr * J_NOT_NULL e) noexcept {
    J_ASSERT(index < num_inputs);
    input(index).expr = e;
    e->parent = this;
    e->parent_pos = index;
    return e;
  }
}
