#pragma once

#include "lisp/assembly/registers.hpp"
#include "util/flags.hpp"
#include "containers/static_array.hpp"

namespace j::lisp::assembly {
  enum class reg_flag : u8_t {
    callee_saved,
    fn_arg,
    fn_result,
    va_fp_count,
    stack_ptr,
    frame_ptr,
    static_chain_ptr,
  };
  using reg_flags = util::flags<reg_flag, u8_t>;

  struct phys_reg_info final {
    phys_reg reg;
    reg_flags flags;
    const char * names[5] = { nullptr };

    J_INLINE_GETTER_NONNULL const char * name() const noexcept {
      return names[0];
    }
  };

  struct reg_info final {
    phys_reg_info gpr_regs[16];
    containers::static_array<phys_reg, 32> caller_saved;
    containers::static_array<phys_reg, 16> callee_saved;
    containers::static_array<phys_reg, 16> gpr_callee_saved;
    containers::static_array<phys_reg, 16> gpr_arg_regs;
    containers::static_array<phys_reg, 16> gpr_result_regs;
    containers::static_array<phys_reg, 16> gpr_alloc_list;

    phys_reg_info fp_regs[16];
    containers::static_array<phys_reg, 16> fp_callee_saved;
    containers::static_array<phys_reg, 16> fp_arg_regs;
    containers::static_array<phys_reg, 16> fp_result_regs;
    containers::static_array<phys_reg, 16> fp_alloc_list;

    reg stack_ptr;
    reg frame_ptr;
    reg static_chain_ptr;
    reg va_fp_ptr;

    [[nodiscard]] inline const phys_reg_info & operator[](phys_reg reg) const noexcept {
      if (reg.is_gpr()) {
        J_ASSERT(gpr_regs[reg.index].reg == reg);
        return gpr_regs[reg.index];
      } else {
        J_ASSERT(reg);
        J_ASSERT(fp_regs[reg.index - 16].reg == reg);
        return fp_regs[reg.index - 16];
      }
    }

    J_INLINE_GETTER_NONNULL const char * name_of(phys_reg reg) const noexcept {
      return operator[](reg).names[0];
    }

    J_INLINE_GETTER const char * name_of(reg reg) const noexcept {
      J_ASSERT(reg.width() != mem_width::none);
      return operator[](reg.phys()).names[((u8_t)reg.width() - 1)];
    }
  };
}
