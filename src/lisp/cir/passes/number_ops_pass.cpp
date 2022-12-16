#include "lisp/cir/passes/cir_passes.hpp"
#include "lisp/cir/ssa/bb.hpp"
#include "lisp/cir/ssa/cir_ssa.hpp"

namespace j::lisp::cir::inline passes {
  void number_ops_pass_fn(const cir_pass_context & ctx) noexcept {
    op_index op_index(0);
    for (auto & bb : ctx.ssa->in_reverse_postorder()) {
      bb.begin_index = op_index;
      for (auto & op : bb) {
        op.index = op_index;
        op_index.to_next();
      }
      bb.end_index = op_index;
    }
    ctx.ssa->num_ops = op_index.instruction_index();
  }

  const cir_pass number_ops_pass{
    number_ops_pass_fn,
    "number-ops", "Number CIR ops sequentially.",
  };
}
