#pragma once

#include "lisp/air/exprs/expr_type.hpp"
#include "lisp/air/values/val_spec.hpp"

namespace j::lisp::air::exprs {
  struct expr;

  /// Information about expression sizes.
  struct expr_size_info final {
    u8_t static_size = 0U;
    bool has_dynamic_inputs = false;
    u8_t num_static_inputs = 0U;
    bool has_var_defs = false;
  };

  /// Table containing information on the size of each `expr`.
  J_A(ND) extern constinit const expr_size_info expr_sizes_v[num_expr_types_v];

  /// Gets the size of `e`, without possible metadata.
  [[nodiscard]] u32_t get_expr_size(const expr * J_NOT_NULL e) noexcept;



  /// Initial value types and reperesentations for `expr`s.
  struct expr_init_data final {
    val_spec result{};
    val_spec args[3]{};

    [[nodiscard]] inline const val_spec & arg(u8_t i) const noexcept {
      return args[min(i, 2)];
    }
  };
}
