#pragma once

#include "hzd/utility.hpp"

#define J_WITH_CIR_OPS(M) \
  M(phi)M(iconst)M(fconst)M(mconst)M(icopy)M(fcopy) \
  M(mov)M(swap)M(push)M(pop) \
  M(fn_enter)M(fn_iarg)M(fn_farg)M(fn_rest_ptr)M(fn_rest_len)M(fn_sc_ptr) \
  M(alloca)M(dealloca)M(call)M(full_call)M(abi_call) \
  M(ineg)M(bnot) \
  M(iadd)M(isub)M(smul)M(sdiv)M(umul)M(udiv)                  \
  M(band)M(bor)M(bxor) \
  M(shl)M(shr)M(sar) \
  M(fadd)M(fsub) \
  M(eq)M(neq) \
  M(tag)M(untag) \
  M(mem_ird)M(mem_iwr)M(mem_frd)M(mem_fwr) \
  M(mem_copy64) \
  M(lea) \
  M(debug_trap)M(iret)M(fret)M(type_error)M(jmp)M(jmpc)M(jmpt)M(jmptype)


namespace j::lisp::cir::inline ops {
  /// Opcodes for `op`.
  enum class op_type : u8_t {
    /// Invalid opcode.
    none = 0,

#define J_ENUM_VALUE(O) O,
    J_WITH_CIR_OPS(J_ENUM_VALUE)
#undef J_ENUM_VALUE

    min_normal_terminal = jmp,
    min_terminal = iret,
    max_op_type = jmptype,
  };

  J_A(ND) constexpr inline u8_t max_op_type_v = (u8_t)op_type::max_op_type;
  J_A(ND) constexpr inline u8_t num_op_types_v = (u8_t)op_type::max_op_type + 1U;

  J_A(AI,NODISC) inline bool is_arg_op(op_type type) noexcept {
    return type >= op_type::fn_iarg && type <= op_type::fn_sc_ptr;
  }
}
