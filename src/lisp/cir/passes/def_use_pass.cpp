#include "lisp/cir/passes/cir_passes.hpp"
#include "lisp/cir/ssa/bb.hpp"
#include "lisp/cir/ssa/cir_ssa.hpp"
#include "lisp/cir/ops/op.hpp"

namespace j::lisp::cir::inline passes {
  namespace {
    void apply_def_use(const cir_pass_context & ctx) {
      for (auto & op : ctx.ssa->ops_in_reverse_postorder()) {
        op.result.next_use = nullptr;
      }
      for (auto & op : ctx.ssa->ops_in_postorder()) {
        for (u8_t i = 0U, max = op.num_inputs; i < max; ++i) {
          auto & in = op.input(i);
          if (in.is_op_result()) {
            auto def = in.op_result_data.use.def;
            in.op_result_data.use.next_use = def->result.next_use;
            def->result.next_use = &in.op_result_data.use;
          } else if (in.is_mem()) {
            if (in.mem_data.base.is_op_result()) {
              auto def = in.mem_data.base.op_result_data.use.def;
              in.mem_data.base.op_result_data.use.next_use = def->result.next_use;
              def->result.next_use = &in.mem_data.base.op_result_data.use;
            }
            if (in.mem_data.index.is_op_result()) {
              auto def = in.mem_data.index.op_result_data.use.def;
              in.mem_data.index.op_result_data.use.next_use = def->result.next_use;
              def->result.next_use = &in.mem_data.index.op_result_data.use;
            }
          }
        }
      }
    }
  }

  const cir_pass def_use_pass{apply_def_use, "def-use", "Reconstruct def-use chains."};
}
