#pragma once

#include "lisp/cir/ssa/bb_op_iterator.hpp"
#include "lisp/cir/ssa/bb_edge.hpp"
#include "lisp/cir/ops/op.hpp"
#include "strings/string_view.hpp"
#include "containers/span.hpp"
#include "hzd/iterators.hpp"

namespace j::lisp::assembly::inline buffers {
  struct label_record;
}

namespace j::lisp::cir::inline ssa {
  struct cir_ssa;

  /// A basic block.
  struct bb final {
    J_BOILERPLATE(bb, CTOR_CE, COPY_DEL)

    /// The first op of this basic block.
    op * first_op = nullptr;
    /// The last op of this basic block.
    ///
    /// \note For finished basic blocks, this will be a terminal (branching) op.
    op * last_op = nullptr;

    /// Index of the first `op` in the basic block.
    ///
    /// \note If the basic block is empty, this will be the index of the last instruction of the last BB.
    op_index begin_index;

    /// Index after the last `op` in the basic block.
    ///
    /// \note If the basic block is empty, this will be the same as `begin_index`.
    op_index end_index;

    /// Index of the basic block, in reverse post order.
    ///
    /// The index is populated in a pass after constructing all basic blocks.
    /// Valid values are non-negative, with -1 used as an indicator for invalid data.
    ///
    /// \note Reverse postorder guarantees that dominators of a basic block will
    ///       have lower indices than the block itself.
    i16_t index = -1;

    /// Number of branches leading to this `bb` from other basic blocks (and from the same for loops.)
    u8_t num_entry_edges = 0;
    /// Number of branches leaving this `bb`.
    u8_t num_exit_edges = 0;

    /// Pointer to the first branch entering the `bb`.
    bb_edge * entry_edges_begin = nullptr;
    /// Pointer to the first branch leaving the `bb`.
    bb_edge * exit_edges_begin = nullptr;

    u8_t num_live_ins = 0U;
    live_range ** live_ins_begin = nullptr;

    /// The label starting the basic block.
    assembly::label_record * asm_label = nullptr;

    /// Name of the BB, used e.g. for naming labels.
    strings::const_string_view name;

    /// Access the edges entering this basic block.
    [[nodiscard]] inline span<bb_edge> entry_edges() noexcept {
      return {entry_edges_begin, num_entry_edges};
    }

    /// Access the edges entering this basic block.
    [[nodiscard]] inline span<const bb_edge> entry_edges() const noexcept {
      return {entry_edges_begin, num_entry_edges};
    }

    /// Access the edges leaving this basic block.
    [[nodiscard]] inline span<bb_edge> exit_edges() noexcept {
      return {exit_edges_begin, num_exit_edges};
    }

    /// Access the edges leaving this basic block.
    [[nodiscard]] inline span<const bb_edge> exit_edges() const noexcept {
      return {exit_edges_begin, num_exit_edges};
    }

    /// Access the live ranges that are live at the start of the block.
    [[nodiscard]] inline span<live_range*> live_ins() const noexcept {
      return {live_ins_begin, num_live_ins};
    }

    [[nodiscard]] inline bb_op_iterator<op, false> begin() noexcept {
      return { first_op, first_op ? first_op->next : nullptr };
    }
    J_INLINE_GETTER bb_op_iterator<op, false> end() noexcept {
      return { nullptr, nullptr };
    }
    [[nodiscard]] inline bb_op_iterator<const op, false> begin() const noexcept {
      return { first_op, first_op ? first_op->next : nullptr  };
    }
    J_INLINE_GETTER bb_op_iterator<const op, false> end() const noexcept {
      return { nullptr, nullptr };
    }

    [[nodiscard]] inline bb_op_iterator<op, true> rbegin() noexcept {
      return { last_op, last_op ? last_op->previous : nullptr  };
    }
    J_INLINE_GETTER bb_op_iterator<op, true> rend() noexcept {
      return { nullptr, nullptr };
    }
    [[nodiscard]] inline bb_op_iterator<const op, true> rbegin() const noexcept {
      return { last_op, last_op ? last_op->previous : nullptr };
    }
    J_INLINE_GETTER bb_op_iterator<const op, true> rend() const noexcept {
      return { nullptr, nullptr };
    }

    [[nodiscard]] reverse_view<bb> in_postorder() noexcept {
      return {this};
    }

    [[nodiscard]] reverse_view<const bb> in_postorder() const noexcept {
      return {this};
    }
  };
}
