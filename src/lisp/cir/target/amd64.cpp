#include "lisp/cir/target/amd64.hpp"
#include "lisp/cir/target/cir_target.hpp"
#include "lisp/cir/ops/op_accessors.hpp"
#include "lisp/cir/cir_context.hpp"
#include "lisp/assembly/register_info.hpp"
#include "lisp/assembly/amd64/registers.hpp"

namespace j::lisp::cir {
  namespace {
    [[nodiscard]] loc_specifier get_fn_arg_loc(const op * J_NOT_NULL op) noexcept{
      auto accessor = as_arg(*op);
      u8_t index = accessor.index();
      if (op->type == op_type::fn_iarg) {
        if (accessor.has_rest_arg()) {
          index += 2U;
        } else if (accessor.has_static_chain()) {
          ++index;
        }
      }
      return loc_specifier(
        loc::from_phys_reg(
          (op->type == op_type::fn_iarg
           ? cir_context->target.reg_info->gpr_arg_regs
           : cir_context->target.reg_info->fp_arg_regs)[index]
          ));
    }

    J_A(ND) inline constexpr loc_mask rax{loc::from_phys_reg(assembly::amd64::preg::rax)};
    J_A(ND) inline constexpr loc_mask rcx{loc::from_phys_reg(assembly::amd64::preg::rcx)};
    J_A(ND) inline constexpr loc_mask rsi{loc::from_phys_reg(assembly::amd64::preg::rsi)};
    J_A(ND) inline constexpr loc_mask rsi_rcx = rsi | rcx;
    J_A(ND) inline constexpr loc_specifier rcx_spec{rcx};
    J_A(ND) inline constexpr loc_specifier rsi_spec{rsi};
    J_A(ND) inline constexpr loc_specifier rax_spec{rax};
    J_A(ND) inline constexpr loc_specifier rdi_spec{loc::from_phys_reg(assembly::amd64::preg::rdi)};
    J_A(ND) inline constexpr loc_mask rdx{loc::from_phys_reg(assembly::amd64::preg::rdx)};
  }

  void initialize_target_amd64(cir_target & tgt) noexcept {
    auto & ops = tgt.ops;
    ops[op_type::fret].params[0].spec = loc_specifier(loc::from_phys_reg(tgt.reg_info->fp_result_regs[0]));
    ops[op_type::fn_farg].result.spec_getter = ops[op_type::fn_iarg].result.spec_getter = get_fn_arg_loc;

    loc_mask clobbered;
    for (auto & reg : tgt.reg_info->caller_saved) {
      clobbered.add(loc::from_phys_reg(reg));
    }

    const auto gpr_arg_regs = tgt.reg_info->gpr_arg_regs;
    const loc_mask gpr_result = loc::from_phys_reg(tgt.reg_info->gpr_result_regs[0]);
    {
      auto & op = ops[op_type::full_call];
      op.clobbered_regs = clobbered;
      ops[op_type::iret].params[0].spec = op.result.spec = gpr_result;

      const loc_mask arg0 = loc::from_phys_reg(gpr_arg_regs[0]), arg1 = loc::from_phys_reg(gpr_arg_regs[1]);
      op.params[0].spec = loc_specifier::any.without(arg0 | arg1).with_hint(gpr_result);
      ops[op_type::fn_rest_ptr].result.spec = op.params[1].spec = arg0;
      ops[op_type::fn_rest_len].result.spec = op.params[2].spec = arg1;
    }


    {
      auto & op = ops[op_type::call];
      op.clobbered_regs = clobbered;
      op.result.spec = gpr_result;
      const loc_mask arg2 = loc::from_phys_reg(gpr_arg_regs[2]);
      ops[op_type::fn_sc_ptr].result.spec = op.params[0].spec = arg2;
      const loc_specifier param_spec = loc_specifier::any.without(arg2);
      for (u32_t i = 1; i < param_info_count_v; ++i) {
        op.params[i].spec = param_spec;
      }
    }

    {
      auto & op = ops[op_type::abi_call];
      op.clobbered_regs = clobbered;
      op.result.spec = gpr_result;
      op.params[0].spec = loc_specifier(loc_mask::any, gpr_result);
      u8_t i = 1;
      for (auto reg : gpr_arg_regs) {
        J_ASSERT(i < param_info_count_v);
        op.params[i++].spec = loc_specifier(loc::from_phys_reg(reg));
      }
    }



    for (auto t : (op_type[]){op_type::sdiv, op_type::udiv}) {
      auto & op = ops[t];
      op.clobbered_regs = rdx;
      op.result.spec = op.params[0].spec = rax_spec;
      op.params[1].spec.remove(rax);
    }

    for (auto t : (op_type[]){op_type::shl, op_type::shr, op_type::sar}) {
      auto & op = ops[t];
      op.params[1].spec = rcx_spec;
      op.result.alias = op.result.alias.not_arg(1);
    }

    {
      auto & op = ops[op_type::mem_copy64];
      op.params[0].spec = op.result.spec = rdi_spec;
      op.params[1].spec = rsi_spec;
      op.params[2].spec = rcx_spec;
      op.clobbered_regs = rsi_rcx;
    }
  }
}
