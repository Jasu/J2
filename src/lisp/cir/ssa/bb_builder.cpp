#include "lisp/cir/ssa/bb_builder.hpp"
#include "lisp/cir/ops/op.hpp"
#include "logging/global.hpp"
#include "lisp/cir/ssa/bb.hpp"

namespace j::lisp::cir::inline ssa {
  void bb_builder::detach_op(struct op * J_NOT_NULL op) noexcept {
    for (op_result_input & r : op->op_results()) {
      r.use.detach();
    }
  }

  void bb_builder::erase_no_detach(struct op * J_NOT_NULL op) noexcept {
    J_ASSUME_NOT_NULL(bb, pool);
    J_ASSERT(bb->last_op && bb->first_op);
    if (!op->next) {
      J_ASSERT(bb->last_op == op);
      bb->last_op = op->previous;
    } else {
      op->next->previous = op->previous;
    }
    if (!op->previous) {
      J_ASSERT(bb->first_op == op);
      bb->first_op = op->next;
    } else {
      op->previous->next = op->next;
    }
  }

  void bb_builder::replace_result(op * J_NOT_NULL target, op * J_NOT_NULL source) noexcept {
    J_ASSUME(target != source);
    J_ASSUME(target->type != op_type::none && source->type != op_type::none);
    J_ASSUME(target->has_result && source->has_result);
    use * last_use = nullptr;
    for (auto use = target->result.next_use; use; use = use->next_use) {
      J_ASSERT(use->def == target);
      use->def = source;
      last_use = use;
    }
    if (last_use) {
      last_use->next_use = source->result.next_use;
      source->result.next_use = target->result.next_use;
    }
    target->result.next_use = nullptr;
  }

  void bb_builder::replace_result(op * J_NOT_NULL target, input source) noexcept {
    J_ASSUME(target->has_result);
    for (auto use = target->result.next_use, next = (use && use->next_use) ? use->next_use : nullptr;
         use;
         use = next, next = (use && use->next_use) ? use->next_use : nullptr) {
      J_ASSERT(use->def == target);
      use->input()->set_value(use->input_index, source);
    }
    // target->result.next_use = &target->result;
  }

  void bb_builder::initialize_edges(op * J_NOT_NULL terminal) const noexcept {
    J_ASSUME_NOT_NULL(bb, pool);
    J_ASSERT(terminal->type >= op_type::min_normal_terminal);
    bb->exit_edges_begin = terminal->exits_begin();
    bb->num_exit_edges = terminal->num_exits;
    J_ASSUME(terminal->num_exits > 0);
    auto edge = bb->exit_edges_begin;
    for (u8_t i = 0u, end = terminal->num_exits; i < end; ++i, ++edge) {
      if (edge->bb) {
        edge->index = edge->bb->num_entry_edges++;
        ::new (edge->bb->entry_edges_begin + edge->index) bb_edge{this->bb, i};
      }
    }
  }

  void bb_builder::replace_with(op * J_NOT_NULL target, op * J_NOT_NULL new_node) noexcept {
    J_ASSUME_NOT_NULL(bb->first_op, bb->last_op);
    if (target->previous) {
      J_ASSUME(bb->first_op != target);
      J_ASSUME(target->previous->next == target);
      target->previous->next = new_node;
      new_node->previous = target->previous;
    } else {
      J_ASSUME(bb->first_op == target);
      bb->first_op = new_node;
    }

    if (target->next) {
      J_ASSUME(bb->last_op != target);
      J_ASSUME(target->next->previous == target);
      target->next->previous = new_node;
      new_node->next = target->next;
    } else {
      J_ASSUME(bb->last_op == target);
      bb->last_op = new_node;
    }

    replace_result(target, new_node);
    if (target->num_inputs) {
      detach_op(target);
    }
  }
  void bb_builder::connect_back(op * J_NOT_NULL result) const noexcept {
    if (bb->last_op) {
      J_ASSERT(bb->first_op);
      bb->last_op->next = result;
      result->previous = bb->last_op;
    } else {
      J_ASSERT(!bb->first_op);
      bb->first_op = result;
    }
    bb->last_op = result;
    if (result->type >= op_type::min_normal_terminal) {
      initialize_edges(result);
    }
  }

  void bb_builder::connect_front(op * J_NOT_NULL result) const noexcept {
    J_ASSERT(bb);
    if (bb->last_op) {
      J_ASSUME_NOT_NULL(bb->first_op);
      bb->first_op->previous = result;
      result->next = bb->first_op;
    } else {
      J_ASSUME(!bb->first_op);
      bb->last_op = result;
    }
    bb->first_op = result;
  }

  void bb_builder::connect_before(op * J_NOT_NULL succ, op * J_NOT_NULL result) const noexcept {
    J_ASSUME_NOT_NULL(bb->first_op, bb->last_op);
    if (succ->previous) {
      J_ASSUME(succ->previous->next == succ);
      result->previous = succ->previous;
      succ->previous->next = result;
    } else {
      J_ASSUME(bb->first_op == succ);
      bb->first_op = result;
    }
    succ->previous = result;
    result->next = succ;
  }

  void bb_builder::connect_after(op * pred, op * J_NOT_NULL result) const noexcept {
    op * next = pred ? pred->next : bb->first_op;
    result->next = next;
    if (next) {
      J_ASSUME(next->previous == pred);
      next->previous = result;
    } else {
      J_ASSUME(bb->last_op == pred);
      bb->last_op = result;
    }
    if (pred) {
      pred->next = result;
      result->previous = pred;
    } else {
      bb->first_op = result;
    }
  }
}
