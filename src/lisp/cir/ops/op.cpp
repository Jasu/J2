#include "lisp/cir/ops/op.hpp"
#include "logging/global.hpp"
#include "lisp/cir/ops/ops.hpp"
#include "mem/bump_pool.hpp"
#include "lisp/common/metadata_init.hpp"

namespace j::lisp::cir::detail {
  [[nodiscard]] J_RETURNS_NONNULL op * create_op(j::mem::bump_pool & pool, u32_t size, op_type type, bool has_result) noexcept {
    // if(type == op_type::jmp) {
    //   J_DEBUG("Alloc op sz {}", size);
    // }
    op * const result = ::new (pool.allocate_aligned_zero(size, 16U)) op {
      .type = type,
      .has_result = has_result,
      .result{
        .type = use_type::op_result,
      },
    };
    result->result.def = result;
    return result;
  }
}
namespace j::lisp::cir::inline ops {
  namespace {
#define J_STATIC_SIZE_ENTRY(OP) [(u8_t)op_type::OP] = defs::OP.static_size_v,
    const u32_t static_sizes[num_op_types_v] = {
      J_WITH_CIR_OPS(J_STATIC_SIZE_ENTRY)
    };
  }

  [[nodiscard]] loc & op::aux_reg(u8_t i) noexcept {
    J_ASSUME(i < num_aux_regs);
    return reinterpret_cast<loc *>(inputs_end())[i];
  }

  [[nodiscard]] loc op::aux_reg(u8_t i) const noexcept {
    return reinterpret_cast<const loc *>(inputs_end())[i];
  }

  void op::set_input_value(u8_t i, const struct input & value) noexcept {
    input(i).set_value(i, value);
  }

  [[nodiscard]] span<struct input> op::inputs() noexcept {
    return { reinterpret_cast<struct input *>(this + 1), num_inputs };
  }

  [[nodiscard]] const span<const struct input> op::inputs() const noexcept {
    return { reinterpret_cast<const struct input *>(this + 1), num_inputs };
  }

  [[nodiscard]] J_RETURNS_NONNULL struct input * op::inputs_end() noexcept {
    return reinterpret_cast<struct input *>(this + 1) + num_inputs;
  }
  [[nodiscard]] J_RETURNS_NONNULL const struct input * op::inputs_end() const noexcept {
    return reinterpret_cast<const struct input *>(this + 1) + num_inputs;
  }

  [[nodiscard]] J_RETURNS_NONNULL bb_edge * op::exits_begin() noexcept {
    return reinterpret_cast<bb_edge *>(align_up(reinterpret_cast<loc *>(inputs_end()) + max_aux_regs, 8U));
  }
  [[nodiscard]] J_RETURNS_NONNULL const bb_edge * op::exits_begin() const noexcept {
    return reinterpret_cast<const bb_edge *>(align_up(reinterpret_cast<const loc *>(inputs_end()) + max_aux_regs, 8U));
  }

  [[nodiscard]] J_RETURNS_NONNULL void * op::static_begin() noexcept {
    return exits_begin() + num_exits;
  }

  [[nodiscard]] J_RETURNS_NONNULL const void * op::static_begin() const noexcept {
    return exits_begin() + num_exits;
  }


  [[nodiscard]] bool op::has_input(cir::op * J_NOT_NULL other) const noexcept {
    for (auto & op_res : op_results()) {
      if (op_res.use.def == other) {
        return true;
      }
    }
    return false;
  }

  [[nodiscard]] struct bb_edge & op::exit(u8_t i) noexcept {
    J_ASSUME(i < num_exits);
    return exits_begin()[i];
  }

  [[nodiscard]] const struct bb_edge & op::exit(u8_t i) const noexcept {
    J_ASSUME(i < num_exits);
    return exits_begin()[i];
  }

  [[nodiscard]] op_result_view<struct input> op::op_results() noexcept {
    auto ins = inputs();
    auto beg = ins.begin(), end = ins.end();
    while (beg != end && (beg->type != input_type::op_result && beg->type != input_type::mem)) {
      ++beg;
    }
    return op_result_view<struct input>{beg, end};
  }

  [[nodiscard]] op_result_view<const struct input> op::op_results() const noexcept {
    auto ins = inputs();
    auto beg = ins.begin(), end = ins.end();
    while (beg != end && (beg->type != input_type::op_result && beg->type != input_type::mem)) {
      ++beg;
    }
    return op_result_view<const struct input>{beg, end};
  }

  [[nodiscard]] metadata_view op::metadata() const noexcept {
    return {reinterpret_cast<const struct metadata*>(
        has_metadata ? align_up(add_bytes(static_begin(), static_sizes[(u8_t)type]), 8) : nullptr)};
  }

  void write_op_metadata(op * J_NOT_NULL to, const metadata_init & md) noexcept {
    to->has_metadata = true;
    md.write(align_up(add_bytes<char*>(to->static_begin(), static_sizes[(u8_t)to->type]), 8));
  }

  template struct op_result_iterator<input>;
  template struct op_result_iterator<const input>;
}
