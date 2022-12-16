#include "lisp/assembly/amd64/abi/ia64_target.hpp"
#include "lisp/assembly/register_info.hpp"
#include "lisp/assembly/amd64/registers.hpp"

#define J_MAKE_FP_REG(IDX, ...) {             \
    preg::ymm##IDX,                           \
    __VA_ARGS__,                              \
    {"ymm" # IDX, "xmm" # IDX, "ymm" # IDX }, \
  }

#define J_MAKE_FP_REG_RESULT(IDX) J_MAKE_FP_REG(IDX, { reg_flag::fn_arg, reg_flag::fn_result })
#define J_MAKE_FP_REG_ARG(IDX) J_MAKE_FP_REG(IDX, reg_flag::fn_arg)
#define J_MAKE_FP_REG_NONE(IDX) J_MAKE_FP_REG(IDX, {})

namespace j::lisp::assembly::amd64::abi {
  namespace {
    J_A(ND) constinit const reg_info info{
      .gpr_regs = {
        { preg::rax,
          { reg_flag::va_fp_count, reg_flag::fn_result },
          {"rax", "al", "ax", "eax", "rax"}},
        { preg::rcx,
          reg_flag::fn_arg,
          {"rcx", "cl", "cx", "ecx", "rcx"}},
        { preg::rdx,
          { reg_flag::fn_arg, reg_flag::fn_result },
          {"rdx", "dl", "dx", "edx", "rdx"}},
        { preg::rbx,
          reg_flag::callee_saved,
          {"rbx", "bl", "bx", "ebx", "rbx"}},
        { preg::rsp,
          reg_flag::stack_ptr,
          {"rsp", "spl", "sp", "esp", "rsp"}},
        { preg::rbp,
          { reg_flag::callee_saved, reg_flag::frame_ptr},
          {"rbp", "bpl", "bp", "ebp", "rbp"}},
        { preg::rsi,
          reg_flag::fn_arg,
          {"rsi", "sil", "si", "esi", "rsi"}},
        { preg::rdi,
          reg_flag::fn_arg,
          {"rdi", "dil", "di", "edi", "rdi"}},
        { preg::r8,
          reg_flag::fn_arg,
          {"r8", "r8b", "r8w", "r8d", "r8"}},
        { preg::r9,
          reg_flag::fn_arg,
          {"r9", "r9b", "r9w", "r9d", "r9"}},
        { preg::r10,
          reg_flag::static_chain_ptr,
          {"r10", "r10b", "r10w", "r10d", "r10"}},
        { preg::r11,
          {},
          {"r11", "r11b", "r11w", "r11d", "r11"}},
        { preg::r12,
          reg_flag::callee_saved,
          {"r12", "r12b", "r12w", "r12d", "r12"}},
        { preg::r13,
          reg_flag::callee_saved,
          {"r13", "r13b", "r13w", "r13d", "r13"}},
        { preg::r14,
          reg_flag::callee_saved,
          {"r14", "r14b", "r14w", "r14d", "r14"}},
        { preg::r15,
          reg_flag::callee_saved,
          {"r15", "r15b", "r15w", "r15d", "r15"}},
      },
      // RBP is omitted, since it is used as frame pointer.
      .caller_saved{
        preg::rax, preg::rcx, preg::rdx, preg::rsi, preg::rdi,
        preg::r8,  preg::r9,  preg::r10, preg::r11,

        preg::ymm0, preg::ymm1, preg::ymm2,  preg::ymm3,  preg::ymm4,  preg::ymm5,  preg::ymm6,  preg::ymm7,
        preg::ymm8, preg::ymm9, preg::ymm10, preg::ymm11, preg::ymm12, preg::ymm13, preg::ymm14, preg::ymm15,
      },
      // RBP is omitted, since it is used as frame pointer.
      .callee_saved{preg::rbx, preg::r12, preg::r13, preg::r14, preg::r15},
      // RBP is omitted, since it is used as frame pointer.
      .gpr_callee_saved{preg::rbx, preg::r12, preg::r13, preg::r14, preg::r15},
      .gpr_arg_regs{preg::rdi, preg::rsi, preg::rdx, preg::rcx, preg::r8, preg::r9},
      .gpr_result_regs{preg::rax, preg::rdx},
      .gpr_alloc_list{
          preg::rcx, // Arg 4
          preg::rdx, // Arg 3  / result 2
          preg::rsi, // Arg 2
          preg::rdi, // Arg 1
          preg::r10, // Static chain
          preg::r9,  // Arg 6
          preg::r8,  // Arg 5
          preg::rax, // Result / VA count
          preg::rbx, // Callee-saved
          preg::r11, // Callee-saved
          preg::r12, // Callee-saved
          preg::r13, // Callee-saved
          preg::r14, // Callee-saved
          preg::r15, // Callee-saved
      },

      .fp_regs = {
        J_MAKE_FP_REG_RESULT(0),
        J_MAKE_FP_REG_RESULT(1),
        J_MAKE_FP_REG_ARG(2),
        J_MAKE_FP_REG_ARG(3),
        J_MAKE_FP_REG_ARG(4),
        J_MAKE_FP_REG_ARG(5),
        J_MAKE_FP_REG_ARG(6),
        J_MAKE_FP_REG_ARG(7),
        J_MAKE_FP_REG_NONE(8),
        J_MAKE_FP_REG_NONE(9),
        J_MAKE_FP_REG_NONE(10),
        J_MAKE_FP_REG_NONE(11),
        J_MAKE_FP_REG_NONE(12),
        J_MAKE_FP_REG_NONE(13),
        J_MAKE_FP_REG_NONE(14),
        J_MAKE_FP_REG_NONE(15),
      },

      // No callee-saved FP regs on AMD64 ABI.
      .fp_callee_saved = {},
      .fp_arg_regs{preg::ymm0, preg::ymm1, preg::ymm2, preg::ymm3, preg::ymm4, preg::ymm5, preg::ymm6, preg::ymm7},
      .fp_result_regs{preg::ymm0, preg::ymm1},
      .fp_alloc_list{
          preg::ymm8,
          preg::ymm9,
          preg::ymm10,
          preg::ymm11,
          preg::ymm12,
          preg::ymm13,
          preg::ymm14,
          preg::ymm15,
          preg::ymm7,
          preg::ymm6,
          preg::ymm5,
          preg::ymm4,
          preg::ymm3,
          preg::ymm2,
          preg::ymm1,
          preg::ymm0,
      },
      .stack_ptr = rsp,
      .frame_ptr = rbp,
      .static_chain_ptr = r10,
      .va_fp_ptr = rax,
    };
  }

  J_A(RNN,NODISC) const reg_info * ia64_target::get_reg_info() const noexcept {
    return &info;
  }

  J_A(ND,NODESTROY) const ia64_target ia64_target::instance;
}
