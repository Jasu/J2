#pragma once

#include "lisp/cir/ssa/bb_builder.hpp"
#include "lisp/cir/ops/op_index.hpp"

namespace j::lisp::cir::inline ssa {
  struct cir_ssa;
  struct ssa_builder final {
    [[nodiscard]] bb_builder emplace_back(strings::const_string_view name, u8_t num_entries = 0U);

    [[nodiscard]] bb_builder create_between(bb * J_NOT_NULL from, bb * J_NOT_NULL to) noexcept;

    void replace_bb_with(bb * J_NOT_NULL old_bb, bb * J_NOT_NULL new_bb);

    [[nodiscard]] bb_builder get_builder_of(op * J_NOT_NULL o) noexcept;

    [[nodiscard]] bb_builder get_builder_at(op_index index) noexcept;

    [[nodiscard]] bb_builder get_builder(bb * J_NOT_NULL b) noexcept {
      return bb_builder{b, &pool};
    }

    cir_ssa * ssa = nullptr;
    j::mem::bump_pool & pool;
  };
}
