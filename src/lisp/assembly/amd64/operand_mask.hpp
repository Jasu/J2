#pragma once

#include "lisp/assembly/operand.hpp"
#include "lisp/assembly/amd64/registers.hpp"
#include "lisp/assembly/amd64/opcodes.hpp"

namespace j::lisp::assembly::amd64 {
  enum class operand_type_mask : u8_t {
    r8      = 0b00010001U,
    m8      = 0b00100001U,
    rm8     = 0b00110001U,
    imm8    = 0b01000001U,

    r16     = 0b00010010U,
    m16     = 0b00100010U,
    rm16    = 0b00110010U,
    imm16   = 0b01000010U,

    r32     = 0b00010100U,
    m32     = 0b00100100U,
    rm32    = 0b00110100U,
    imm32   = 0b01000100U,

    r64     = 0b00011000U,
    m64     = 0b00101000U,
    rm64    = 0b00111000U,
    imm64   = 0b01001000U,

    mem     = 0b00101111U,
    imm     = 0b01000000U,
    any_imm = 0b01001111U,

    xmm     = 0b10000001U,
    xmm_m64 = 0b10101001U,
    xmm_m32 = 0b10100101U,
    ymm     = 0b10000010U,

    any_reg = 0b00011111U,
    none    = 0b10001000U,
    never   = 0b01000001U,
  };


  enum class operand_type_special_case : u32_t {
    none = 0b111111U << 8,
    rax  = 0b111110U << 8,
    rcx  = 0b111101U << 8,
    rdx  = 0b111011U << 8,
    rdi  = 0b110111U << 8,
    rsi  = 0b101111U << 8,
    one  = 0b011111U << 8,
    never  = 0U,
  };

  struct single_operand_mask final {
    u32_t mask = (u32_t)operand_type_mask::none | ((u32_t)operand_type_special_case::none);

    J_ALWAYS_INLINE constexpr single_operand_mask() noexcept = default;

    J_ALWAYS_INLINE constexpr explicit single_operand_mask(u32_t mask) noexcept
      : mask(mask)
    { }

    J_ALWAYS_INLINE constexpr single_operand_mask(
      operand_type_mask type_mask,
      operand_type_special_case special_case = operand_type_special_case::none) noexcept
      : mask((u32_t)type_mask | (u32_t)special_case)
      { }

    constexpr single_operand_mask(const operand & op) noexcept {
      if (op.is_mem() || op.is_imm()) {
        mask = (u8_t)op.type();
      } else if (op.is_rel()) {
        mask = mem_bit_v | ((u8_t)op.type() & 0xFU);
      } else {
        J_ASSERT(op.is_reg());
        if (op.reg().is_fp()) {
          mask = (u32_t)(op.width() == mem_width::vec128
                         ? operand_type_mask::xmm
                         : operand_type_mask::ymm);
        } else {
          mask = 0b00010000U | (u32_t)(mem_width_mask(op.width()));
        }
      }

      // Special cases
      if (op.is_imm()) {
        mask |= op.imm() == 1LL
          ? (u32_t)operand_type_special_case::one
          : (u32_t)operand_type_special_case::none;
      }
      if (op.is_reg()) {
        auto reg = op.reg();
        if (reg.is(preg::rax)) {
          mask |= (u32_t)operand_type_special_case::rax;
          return;
        }
        if (reg.is(preg::rcx)) {
          mask |= (u32_t)operand_type_special_case::rcx;
          return;
        }
        if (reg.is(preg::rdx)) {
          mask |= (u32_t)operand_type_special_case::rdx;
          return;
        }
        if (reg.is(preg::rdi)) {
          mask |= (u32_t)operand_type_special_case::rdi;
          return;
        }
        if (reg.is(preg::rsi)) {
          mask |= (u32_t)operand_type_special_case::rsi;
          return;
        }
      }
      mask |= (u32_t)operand_type_special_case::none;
    }
  };

  constexpr inline auto single_mask_never = single_operand_mask{operand_type_mask::never, operand_type_special_case::never};
  constexpr inline auto special_case_one = single_operand_mask{operand_type_mask::any_imm, operand_type_special_case::one};

  constexpr inline auto special_case_rax = single_operand_mask{operand_type_mask::r64, operand_type_special_case::rax};
  constexpr inline auto special_case_eax = single_operand_mask{operand_type_mask::r32, operand_type_special_case::rax};
  constexpr inline auto special_case_ax  = single_operand_mask{operand_type_mask::r16, operand_type_special_case::rax};
  constexpr inline auto special_case_al  = single_operand_mask{operand_type_mask::r8,  operand_type_special_case::rax};
  constexpr inline auto special_case_any_rax = single_operand_mask{operand_type_mask::any_reg, operand_type_special_case::rax};

  constexpr inline auto special_case_rcx = single_operand_mask{operand_type_mask::r64, operand_type_special_case::rcx};
  constexpr inline auto special_case_ecx = single_operand_mask{operand_type_mask::r32, operand_type_special_case::rcx};
  constexpr inline auto special_case_cx  = single_operand_mask{operand_type_mask::r16, operand_type_special_case::rcx};
  constexpr inline auto special_case_cl  = single_operand_mask{operand_type_mask::r8,  operand_type_special_case::rcx};
  constexpr inline auto special_case_any_rcx = single_operand_mask{operand_type_mask::any_reg, operand_type_special_case::rcx};

  constexpr inline auto special_case_rdx = single_operand_mask{operand_type_mask::r64, operand_type_special_case::rdx};
  constexpr inline auto special_case_edx = single_operand_mask{operand_type_mask::r32, operand_type_special_case::rdx};
  constexpr inline auto special_case_dx  = single_operand_mask{operand_type_mask::r16, operand_type_special_case::rdx};
  constexpr inline auto special_case_dl  = single_operand_mask{operand_type_mask::r8,  operand_type_special_case::rdx};
  constexpr inline auto special_case_any_rdx = single_operand_mask{operand_type_mask::any_reg, operand_type_special_case::rdx};

  constexpr inline auto special_case_any_rdi = single_operand_mask{operand_type_mask::any_reg, operand_type_special_case::rdi};
  constexpr inline auto special_case_any_rsi = single_operand_mask{operand_type_mask::any_reg, operand_type_special_case::rsi};

  struct operand_mask final {
    u64_t mask;
    J_ALWAYS_INLINE explicit constexpr operand_mask(u64_t mask) noexcept
      : mask(mask)
    { }

    constexpr operand_mask(single_operand_mask op0 = {},
                           single_operand_mask op1 = {},
                           single_operand_mask op2 = {},
                           single_operand_mask op3 = {}) noexcept
      : mask((u64_t)op0.mask | (u64_t)op1.mask << 16 | (u64_t)op2.mask << 32 | (u64_t)op3.mask << 48)
    {
    }

    J_INLINE_GETTER constexpr bool operator==(const operand_mask &) const noexcept = default;
  };

  constexpr inline operand_mask operand_mask_never{single_mask_never, single_mask_never, single_mask_never, single_mask_never};

  constexpr operand_mask make_shift_by_one_mask(single_operand_mask op0) noexcept {
    return operand_mask(op0, special_case_one);
  }
}
