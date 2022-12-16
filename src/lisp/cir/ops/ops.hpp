#pragma once

#include "lisp/cir/ops/conditions.hpp"
#include "lisp/cir/ops/op_def.hpp"
#include "lisp/values/imm_type.hpp"
#include "lisp/common/mem_width.hpp"
#include "mem/memory_region.hpp"

#define J_CIR_PARAM_
#define J_CIR_PARAM_IN        detail::input_def_param
#define J_CIR_PARAM_INS       detail::inputs_def
#define J_CIR_PARAM_F32       detail::static_def<float>
#define J_CIR_PARAM_U32       detail::static_def<u32_t>
#define J_CIR_PARAM_U64       detail::static_def<u64_t>
#define J_CIR_PARAM_BOOL      detail::static_def<bool>
#define J_CIR_PARAM_LOC       detail::static_def<loc>
#define J_CIR_PARAM_OP_INPUT  detail::static_def<op_result_input>
#define J_CIR_PARAM_COND      detail::static_def<condition>
#define J_CIR_PARAM_IMM_TYPE  detail::static_def<imm_type>
#define J_CIR_PARAM_IMM_TYPES detail::static_def<imm_type_mask>
#define J_CIR_PARAM_MEM_WIDTH detail::static_def<mem_width>
#define J_CIR_PARAM_MEM_REG   detail::static_def<j::mem::const_memory_region>
#define J_CIR_PARAM_EXIT      detail::exit_def

#define J_CIR_PARAM_AUX_REG       detail::aux_reg_def

#define J_CIR_PARAM(TYPE) J_CIR_PARAM_##TYPE

#define J_CIR_STMT(TYPE, ...) \
  constexpr inline detail::op_def<op_type::TYPE, false __VA_OPT__(,) J_FEC(J_CIR_PARAM, __VA_ARGS__)> TYPE{}

#define J_CIR_EXPR(TYPE, ...)                      \
  constexpr inline detail::op_def<op_type::TYPE, true __VA_OPT__(,) J_FEC(J_CIR_PARAM, __VA_ARGS__)> TYPE{}

namespace j::lisp::cir::inline ops {
  namespace defs {
    template<op_type Type>
    constexpr inline bool op_def_v = false;

    J_CIR_EXPR(iconst, U64);
    J_CIR_EXPR(fconst, F32);
    J_CIR_EXPR(mconst, MEM_REG);

    J_CIR_EXPR(phi, INS);

    J_CIR_EXPR(icopy, IN);
    J_CIR_EXPR(fcopy, IN);
    J_CIR_STMT(mov, OP_INPUT);
    J_CIR_STMT(push, MEM_WIDTH, LOC);
    J_CIR_STMT(pop, MEM_WIDTH, LOC);
    J_CIR_STMT(swap, LOC, LOC);

    J_CIR_EXPR(fn_iarg, U32, BOOL, BOOL);
    J_CIR_EXPR(fn_farg, U32);
    J_CIR_STMT(fn_enter);
    J_CIR_EXPR(fn_rest_ptr);
    J_CIR_EXPR(fn_rest_len);
    J_CIR_EXPR(fn_sc_ptr);
    J_CIR_EXPR(alloca, INS);
    J_CIR_STMT(dealloca, IN);
    J_CIR_EXPR(call, IN, INS);
    J_CIR_EXPR(full_call, IN, IN, IN);
    J_CIR_EXPR(abi_call, IN, INS);

    J_CIR_EXPR(ineg, IN);
    J_CIR_EXPR(iadd, IN, IN);
    J_CIR_EXPR(isub, IN, IN);
    J_CIR_EXPR(smul, IN, IN);
    J_CIR_EXPR(sdiv, IN, IN);
    J_CIR_EXPR(umul, IN, IN);
    J_CIR_EXPR(udiv, IN, IN);

    J_CIR_EXPR(fadd, IN, IN);
    J_CIR_EXPR(fsub, IN, IN);

    J_CIR_EXPR(shl, IN, IN);
    J_CIR_EXPR(shr, IN, IN);
    J_CIR_EXPR(sar, IN, IN);

    J_CIR_EXPR(band, IN, IN);
    J_CIR_EXPR(bor, IN, IN);
    J_CIR_EXPR(bxor, IN, IN);
    J_CIR_EXPR(bnot, IN);

    J_CIR_EXPR(eq, IN, IN);
    J_CIR_EXPR(neq, IN, IN);

    J_CIR_EXPR(tag, IMM_TYPE, IN);
    J_CIR_EXPR(untag, IMM_TYPE, IN);

    J_CIR_EXPR(mem_ird, IN);
    J_CIR_STMT(mem_iwr, IN, IN);
    J_CIR_EXPR(mem_frd, IN);
    J_CIR_STMT(mem_fwr, IN, IN);
    J_CIR_STMT(mem_copy64, IN, IN, IN);

    J_CIR_EXPR(lea, IN);

    J_CIR_STMT(debug_trap);

    J_CIR_STMT(type_error, IMM_TYPES);
    J_CIR_STMT(iret, IN);
    J_CIR_STMT(fret, IN);

    J_CIR_STMT(jmp, EXIT);
    J_CIR_STMT(jmpt, IN, EXIT, EXIT);
    J_CIR_STMT(jmpc, COND, IN, IN, EXIT, EXIT);
    J_CIR_STMT(jmptype, IN, IMM_TYPES, AUX_REG, EXIT, EXIT);
  }

#define J_VISIT_CASE(TYPE) case op_type::TYPE: return defs::TYPE.visit(o, visitor);

  template<typename Visitor>
  J_NO_DEBUG decltype(auto) visit_cir_op(const op * J_NOT_NULL o, Visitor & visitor) {
    switch (o->type) {
      J_WITH_CIR_OPS(J_VISIT_CASE)
    case op_type::none:
      J_UNREACHABLE();
    }
  }
}

#undef J_CIR_PARAM_
#undef J_CIR_PARAM_IN
#undef J_CIR_PARAM_INS
#undef J_CIR_PARAM_F32
#undef J_CIR_PARAM_U32
#undef J_CIR_PARAM_U64
#undef J_CIR_PARAM_BOOL
#undef J_CIR_PARAM_LOC
#undef J_CIR_PARAM_COND
#undef J_CIR_PARAM_IMM_TYPE
#undef J_CIR_PARAM_IMM_TYPES
#undef J_CIR_PARAM_MEM_WIDTH
#undef J_CIR_PARAM_MEM_REG
#undef J_CIR_PARAM_AUX_REG
#undef J_CIR_PARAM_EXIT
#undef J_CIR_PARAM

#undef J_CIR_EXPR
#undef J_CIR_STMT

#undef J_VISIT_CASE
