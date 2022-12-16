#pragma once

#include "lisp/assembly/amd64/operand_mask.hpp"

namespace j::lisp::assembly::amd64 {
  /// Format of the opcode part of the instruction, excluding mandatory prefixes.
  enum class opcode_format : u8_t {
    /// Single-byte opcode, with no register encoded in the opcode itself.
    plain_1,
    /// Two-byte opcode, with no register encoded in the opcode itself.
    plain_2,
    /// Three-byte opcode, with no register encoded in the opcode itself.
    plain_3,
    /// Single-byte opcode with register index in the last three bits.
    plus_reg_1,
    /// Two-byte opcode with register index in the last three bits.
    plus_reg_2,
  };

  constexpr inline u8_t imm_format_imm_bit_v = 0b00010000U;
  constexpr inline u8_t imm_format_rel_bit_v = 0b00100000U;

  enum class imm_format : u8_t {
    none = 0U,

    imm8  = imm_format_imm_bit_v | width_byte_bit_v,
    imm16 = imm_format_imm_bit_v | width_word_bit_v,
    imm32 = imm_format_imm_bit_v | width_dword_bit_v,
    imm64 = imm_format_imm_bit_v | width_qword_bit_v,

    rel8  = imm_format_rel_bit_v | width_byte_bit_v,
    rel16 = imm_format_rel_bit_v | width_word_bit_v,
    rel32 = imm_format_rel_bit_v | width_dword_bit_v,
    rel64 = imm_format_rel_bit_v | width_qword_bit_v,
  };

  /// Mandatory prefix, preceding possible REX prefix.
  enum class mandatory_prefix : u8_t {
    none,
    has_66,
    has_f2,
    has_f3,
  };

  enum class operand_format : u8_t {
    none,

    implicit,
    ri,
    ri_implicit,
    implicit_ri,
    implicit_m,
    implicit2_m,

    m,
    m_implicit,
    mr_implicit,
    r,
    mr,
    rm,
  };

  struct instruction_encoding final {
    operand_mask mask = operand_mask(U64_MAX);

    /// Mandatory prefix that precedes REX prefix.
    enum mandatory_prefix mandatory_prefix:2 = mandatory_prefix::none;
    /// Whether the REX.W  bit is set.
    bool has_rex_w:1 = false;
    enum operand_format operand_format:4 = operand_format::none;
    enum opcode_format opcode_format:3 = opcode_format::plain_1;
    u8_t modrm_forced_r:3 = 0U;
    enum imm_format imm_format:6 = imm_format::none;
    u8_t opcode[4] = {0U};
    const char * name = "";
  };

  struct instruction_encodings final {
    const instruction_encoding * encodings;
    const char * name;

    [[nodiscard]] inline const instruction_encoding * find_encoding(operand_mask op_mask) const noexcept {
      const instruction_encoding * enc = encodings;
      for (;(enc->mask.mask & op_mask.mask) != op_mask.mask; ++enc) { }
      return enc->mask.mask != U64_MAX ? enc : nullptr;
    }
  };

  template<u32_t N>
  struct instruction final {
    const char * name;
    const instruction_encoding encodings[N + 1U];
    operator instruction_encodings() const noexcept {
      return instruction_encodings{encodings, name};
    }
  };
}
