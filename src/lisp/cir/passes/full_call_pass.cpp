#include "lisp/cir/passes/cir_passes.hpp"
#include "lisp/cir/ops/ops.hpp"
#include "lisp/cir/ops/op_accessors.hpp"
#include "lisp/cir/ssa/ssa_builder.hpp"
#include "lisp/cir/ssa/cir_ssa.hpp"
#include "lisp/cir/ssa/bb.hpp"
#include "lisp/compilation/context.hpp"
#include "lisp/functions/func_info.hpp"

namespace j::lisp::cir::inline passes {
  namespace {
    namespace o = ops::defs;
    void apply_full_call_args(const cir_pass_context & ctx) {
      J_ASSERT(ctx.calling_convention == functions::calling_convention::full_call);
      auto info = ctx.func_info;
      u16_t num_args = info->params.num_params;
      J_ASSUME_NOT_NULL(info);
      u16_t num_required_args = num_args - info->params.has_rest;
      if (!num_required_args) {
        return;
      }
      cir::op *ptr_op = nullptr, *ptr_src_op = nullptr, *sz_op = nullptr, *sz_src_op = nullptr;
      for (auto & bb : ctx.ssa_builder->ssa->in_reverse_postorder()) {
        for (auto & op : bb) {
          auto builder = ctx.ssa_builder->get_builder(&bb);
          switch (op.type) {
          case op_type::fn_farg: {
            J_TODO();
          }
          case op_type::fn_iarg: {
            // Emplace the node on first use - it should be the first op in the graph,
            // EXCEPT for any prologue like %fn-enter. This ensures that such nonsense is skipped.
            if (!ptr_op) {
              ptr_op = builder.emplace_before(&op, defs::fn_rest_ptr, copy_metadata(op.metadata()));
              if (!num_required_args) {
                ptr_src_op = ptr_op;
              }
            }
            auto index = as_arg(op).index();
            J_ASSUME(index < num_args);
            builder.replace(&op, defs::mem_ird, copy_metadata(op.metadata()), qword_ptr(ptr_op, index * 8));
            break;
          }
          case op_type::fn_rest_ptr: {
            // Emplace the node on first use - it should be the first op in the graph,
            // EXCEPT for any prologue like %fn-enter. This ensures that such nonsense is skipped.
            if (&op == ptr_op) {
              break;
            }
            if (!ptr_op) {
              ptr_src_op = ptr_op = &op;
              if (num_required_args) {
                ptr_src_op = builder.emplace_after(&op, defs::iadd, copy_metadata(op.metadata()), const_nil, untagged_const_int(num_required_args * 8));
                builder.replace_result(&op, ptr_src_op);
                ptr_src_op->set_input_value(0, ptr_op);
              }
              break;
            }

            if (num_required_args && !ptr_src_op) {
              ptr_src_op = builder.emplace_after(ptr_op, defs::iadd, copy_metadata(ptr_op->metadata()), ptr_op, untagged_const_int(num_required_args * 8));
            }

            if (ptr_op != &op) {
              builder.replace_result(&op, ptr_src_op);
              builder.erase(&op);
            }
            break;
          }
          case op_type::fn_rest_len: {
            if (!sz_op) {
              sz_src_op = sz_op = &op;
              if (num_required_args) {
                sz_src_op = builder.emplace_after(&op, defs::isub, copy_metadata(op.metadata()), const_nil, untagged_const_int(num_required_args));
                builder.replace_result(&op, sz_src_op);
                sz_src_op->set_input_value(0, &op);
              }
              break;
            }
            if (num_required_args && !sz_src_op) {
              sz_src_op = builder.emplace_after(sz_op, defs::isub, copy_metadata(sz_op->metadata()), sz_op, untagged_const_int(num_required_args));
            }

            if (sz_op != &op) {
              builder.replace_result(&op, sz_src_op);
              builder.erase(&op);
            }
            break;
          }
          default:
            break;
          }
        }
      }
    }
  }

  const cir_pass full_call_pass{
    apply_full_call_args, functions::calling_conventions_t{functions::calling_convention::full_call},
    "full-call-args", "Apply full call convention to args",
  };
}
