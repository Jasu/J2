#pragma once

#include "lisp/cir/ops/op_type.hpp"
#include "lisp/cir/ops/op_index.hpp"
#include "lisp/cir/ops/input.hpp"
#include "lisp/cir/ssa/bb_edge.hpp"
#include "lisp/cir/liveness/live_range_fwd.hpp"

namespace j::lisp::inline common{
  struct metadata_view;
  struct metadata_init;
}
namespace j::lisp::cir::inline ops {
  template<typename Input> struct op_result_view;

  /// Layout of an op in memory:
  ///
  /// ```
  /// +==============+
  /// | struct op    |
  /// | - type       | The opcode of the instruction
  /// | - sizes      | Sizes of the trailing regions
  /// | - prev/next  | Pointers to neighboring ops within the basic block
  /// | - first use  | Pointer to the first input using the ops result.
  /// | - result loc | Currently, the result loc and use is also stored for resultless ops.
  /// +==============+
  /// | input 1      | Inputs are the "actual" operands of the op - they must come
  /// +----  ...  ---+ from results of other ops.
  /// | input N      |
  /// +==============+
  /// | aux reg 1    | Auxiliary registers are needed by some ops that compile to
  /// +----  ...  ---+ multiple instructions. The register allocator will allocate
  /// | aux reg N    | them along with inputs and op results, but they are allocated
  /// +----  ...  ---+ only for the duration of the instruction.
  /// | pad to 8B    |
  /// +==============+
  /// | exit 1       | For terminal nodes, exit edges are saved here.
  /// +----  ...  ---+
  /// | pad to 8B    |
  /// +==============+
  /// | static 1     | Static parameters specific to the instruction. E.g. constant
  /// +----  ...  ---+ loads store their constant value from a static operand, and
  /// | static N     | conditional jumps store the condition here.
  /// +==============+
  /// ```
  struct op final {
    /// The "opcode".
    op_type type = op_type::none;
    /// Number of input operands. For each, an `input` is stored after the `op`.
    ///
    /// \note Usually this is constant for each op, but may vary. E.g. function call
    ///       related opcodes have an input for each argument.
    u8_t num_inputs = 0;
    /// Whether the op has a result.
    bool has_result:1 = false;
    /// Number of auxiliary regs. For each, a `loc` is stored after inputs.
    u8_t num_aux_regs:2 = 0U;
    /// Max auxiliary regs.
    u8_t max_aux_regs:2 = 0U;
    bool has_metadata:1 = false;
    /// Number of exits, or zero if not terminal.
    u8_t num_exits = 0U;

    /// Index of the instruction.
    op_index index;

    /// Next op in the basic block, null if last.
    op * next = nullptr;
    /// Previous op in the basic block, null if first.
    op * previous = nullptr;

    /// First use of the result value (if any.)
    use result{};
    imm_type_mask result_types{};


    live_range live_range;

    J_INLINE_GETTER struct input & input(u8_t i) noexcept {
      return reinterpret_cast<struct input *>(this + 1)[i];
    }

    J_INLINE_GETTER const struct input & input(u8_t i) const noexcept {
      return reinterpret_cast<const struct input *>(this + 1)[i];
    }

    void set_input_value(u8_t i, const struct input & value) noexcept;

    J_A(PURE,NODISC) span<struct input> inputs() noexcept;

    J_A(PURE,NODISC) const span<const struct input> inputs() const noexcept;

    J_INLINE_GETTER struct input * inputs_begin() noexcept {
      return reinterpret_cast<struct input *>(this + 1);
    }

    J_INLINE_GETTER const struct input * inputs_begin() const noexcept {
      return reinterpret_cast<const struct input *>(this + 1);
    }

    [[nodiscard]] bool has_input(cir::op * J_NOT_NULL other) const noexcept;

    [[nodiscard]] loc & aux_reg(u8_t i) noexcept;

    [[nodiscard]] loc aux_reg(u8_t i) const noexcept;

    J_A(PURE,RNN,NODISC) void * static_begin() noexcept;

    J_A(PURE,RNN,NODISC) const void * static_begin() const noexcept;

    J_A(PURE,RNN,NODISC) struct input * inputs_end() noexcept;

    J_A(PURE,RNN,NODISC) const struct input * inputs_end() const noexcept;

    J_A(PURE,RNN,NODISC) bb_edge * exits_begin() noexcept;

    J_A(PURE,RNN,NODISC) const bb_edge * exits_begin() const noexcept;

    [[nodiscard]] metadata_view metadata() const noexcept;

    J_A(PURE,NODISC) struct bb_edge & exit(u8_t i) noexcept;

    J_A(PURE,NODISC) const struct bb_edge & exit(u8_t i) const noexcept;

    [[nodiscard]] op_result_view<struct input> op_results() noexcept;

    [[nodiscard]] op_result_view<const struct input> op_results() const noexcept;
  };

  template<typename Input>
  struct op_result_iterator final {
    Input * in;
    Input * max;
    bool is_index;

    op_result_iterator & operator++() noexcept {
      if (in->type == input_type::mem) {
        if (!is_index && in->mem_data.index.is_op_result()) {
          is_index = true;
          return *this;
        }
        is_index = false;
      }
      do {
        ++in;
      } while (in != max && in->type != input_type::op_result && in->type != input_type::mem);
      return *this;
    }

    op_result_iterator operator++(int) noexcept {
      auto result{*this};
      operator++();
      return result;
    }

    J_RETURNS_NONNULL auto * operator->() const noexcept {
      if (in->type == input_type::mem) {
        return is_index ? &in->mem_data.index.op_result_data : &in->mem_data.base.op_result_data;
      }
      return &in->op_result_data;
    }

    J_INLINE_GETTER auto & operator*() const noexcept {
      return *operator->();
    }

    J_INLINE_GETTER bool operator==(const op_result_iterator & rhs) const noexcept {
      return in == rhs.in && is_index == rhs.is_index;
    }
  };

  template<typename Input>
  struct op_result_view final {
    Input *input_begin, *input_end;

    op_result_iterator<Input> begin() const noexcept {
      return {input_begin, input_end, false};
    }

    op_result_iterator<Input> end() const noexcept {
      return {input_end, input_end, false};
    }
  };

  void write_op_metadata(op * J_NOT_NULL to, const metadata_init & md) noexcept;

  extern template struct op_result_iterator<input>;
  extern template struct op_result_iterator<const input>;
}
