#include "lisp/cir/passes/cir_passes.hpp"
#include "lisp/cir/ssa/ssa_builder.hpp"
#include "lisp/cir/ssa/bb.hpp"
#include "lisp/cir/ssa/cir_ssa.hpp"

namespace j::lisp::cir::inline passes {
  extern void reverse_postorder_pass_fn(const cir_pass_context & ctx);
  extern void number_ops_pass_fn(const cir_pass_context & ctx);
  namespace {
    void remove_empty_bbs_pass_fn(const cir_pass_context & ctx) {
      cir_ssa * ssa = ctx.ssa_builder->ssa;
      i32_t num = ssa->size;
      J_ASSUME(num > 0);
      bool did_remove = false;
      for (auto & bb : ssa->in_postorder()) {
        J_ASSUME(bb.first_op && bb.last_op);
        if (bb.num_entry_edges == 0 || bb.num_exit_edges == 0) {
          continue;
        }
        if (bb.last_op->type == op_type::jmp && (!bb.last_op->previous || bb.last_op->previous->type == op_type::phi)) {
          ctx.ssa_builder->replace_bb_with(&bb, bb.exit_edges()[0].bb);
          did_remove = true;
        }
      }
      if (did_remove) {
        reverse_postorder_pass_fn(ctx);
        number_ops_pass_fn(ctx);
      }
    }
  }

  const cir_pass remove_empty_bbs_pass{
    remove_empty_bbs_pass_fn,
    "remove-empty-bbs", "Prune empty basic blocks",
  };
}
