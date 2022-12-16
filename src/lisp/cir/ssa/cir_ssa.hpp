#pragma once

#include "lisp/cir/ssa/ssa_op_iterator.hpp"

namespace j::lisp::cir::inline ssa {
  template<typename BbView>
  struct ssa_op_view final {
    BbView bb_view;

    J_A(ND,NODISC) auto begin() const noexcept {
      return basic_ssa_op_iterator{bb_view, false};
    }

    J_A(ND,NODISC) auto end() const noexcept {
      return basic_ssa_op_iterator{bb_view, true};
    }
  };

  struct cir_ssa final {
    /// The basic block that is the entry to the function.
    ///
    /// \note This is the "authorative" pointer to the actual content of the SSA. The
    ///       reverse post order ordering is only constructed after.
    bb * entry_bb = nullptr;

    /// The `fn_enter` instruction.
    ///
    /// \note Used as insertion point for instructions that have to be in place
    ///       before any other code is run, but after a stack frame exists.
    ///       For example, callee-saved variables are spilled immediately after
    ///       the stack frame is established.
    op * fn_enter_op = nullptr;

    /// The basic block containing the function return statement.
    bb * return_bb = nullptr;
    /// The `fn_ret` instruction.
    op * fn_ret_op = nullptr;

    /// Number of all `op`s.
    i32_t num_ops = -1;

    /// The basic blocks ordered in reverse postorder.
    ///
    /// \note This is filled in a pass, so it is initially empty.
    bb** reverse_postorder_ptr = nullptr;

    /// The number of basic blocks in the computed reverse postorder array.
    u32_t reverse_postorder_size = 0U;

    /// The number of basic blocks in the SSA.
    u16_t size = 0U;

    /// Number of spill slots used (valid after reg alloc)
    u16_t num_spill_slots = 0U;

    /// Returns a view of the basic blocks that can be iterated in postorder.
    ///
    /// Post order iterates from the last executed basic block to the one executed
    /// first, i.e. in reverse order, unlike reverse post order.
    ///
    /// This function requires that the reverse post order has been computed in a pass.
    [[nodiscard]] J_NO_DEBUG bb_iterator_view<bb, true> in_postorder() noexcept {
      return {reverse_postorder_ptr, reverse_postorder_ptr + reverse_postorder_size};
    }

    /// Returns a view of the basic blocks that can be iterated in postorder.
    ///
    /// Post order iterates from the last executed basic block to the one executed
    /// first, i.e. in reverse order, unlike reverse post order.
    ///
    /// This function requires that the reverse post order has been computed in a pass.
    [[nodiscard]] J_NO_DEBUG bb_iterator_view<const bb, true> in_postorder() const noexcept {
      return {reverse_postorder_ptr, reverse_postorder_ptr + reverse_postorder_size};
    }

    /// Returns a view of the basic blocks that can be iterated in postorder.
    ///
    /// Reverse post order iterates from the first executed basic block to the one executed
    /// last, i.e. not in reverse order (unlike post order).
    ///
    /// Reverse post order guarantees that blocks dominating others will be iterated
    /// before them.
    ///
    /// This function requires that the reverse post order has been computed in a pass.
    [[nodiscard]] J_NO_DEBUG bb_iterator_view<bb, false> in_reverse_postorder() noexcept {
      return {reverse_postorder_ptr, reverse_postorder_ptr + reverse_postorder_size};
    }

    /// Returns a view of the basic blocks that can be iterated in postorder.
    ///
    /// Reverse post order iterates from the first executed basic block to the one executed
    /// last, i.e. not in reverse order (unlike post order).
    ///
    /// Reverse post order guarantees that blocks dominating others will be iterated
    /// before them.
    ///
    /// This function requires that the reverse post order has been computed in a pass.
    [[nodiscard]] J_NO_DEBUG bb_iterator_view<const bb, false> in_reverse_postorder() const noexcept {
      return {reverse_postorder_ptr, reverse_postorder_ptr + reverse_postorder_size};
    }

    [[nodiscard]] inline J_NO_DEBUG auto ops_in_reverse_postorder() noexcept {
      return ssa_op_view<bb_iterator_view<bb, false>>{in_reverse_postorder()};
    }
    [[nodiscard]] inline J_NO_DEBUG auto ops_in_reverse_postorder() const noexcept {
      return ssa_op_view<bb_iterator_view<const bb, false>>{in_reverse_postorder()};
    }
    [[nodiscard]] inline J_NO_DEBUG auto ops_in_postorder() noexcept {
      return ssa_op_view<bb_iterator_view<bb, true>>{in_postorder()};
    }
    [[nodiscard]] inline J_NO_DEBUG auto ops_in_postorder() const noexcept {
      return ssa_op_view<bb_iterator_view<const bb, true>>{in_postorder()};
    }
  };
}
