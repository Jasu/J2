#pragma once

#include "lisp/assembly/registers.hpp"
#include "lisp/assembly/linking/reloc.hpp"
#include "bits/bitops.hpp"
#include "lisp/common/mem_scale.hpp"

namespace j::lisp::assembly {
  inline constexpr u8_t width_byte_bit_v  = 0b0001U;
  inline constexpr u8_t width_word_bit_v  = 0b0010U;
  inline constexpr u8_t width_dword_bit_v = 0b0100U;
  inline constexpr u8_t width_qword_bit_v = 0b1000U;

  inline constexpr u8_t width_mask_v  = 0b1111U;

  J_A(AI,ND,NODISC) constexpr u32_t mem_width_mask(mem_width w) noexcept {
    return w == mem_width::none ? 0U : (1U << ((u8_t)w - 1));
  }

  enum class operand_type : u8_t {
    none          = 0b00000000U,
    reg           = 0b00010000U,

    mem8          = 0b00100001U,
    mem16         = 0b00100010U,
    mem32         = 0b00100100U,
    mem64         = 0b00101000U,

    /// 8-bit immediate.
    ///
    /// \note Signedness depends on context.
    imm8          = 0b01000001U,
    /// 16-bit immediate
    ///
    /// \note Signedness depends on context.
    imm16         = 0b01000010U,
    /// 32-bit immediate
    ///
    /// \note Signedness depends on context.
    imm32         = 0b01000100U,
    /// 64-bit immediate
    ///
    /// \note Signedness depends on context.
    imm64         = 0b01001000U,

    /// PC-relative signed 8-bit
    rel8          = 0b10000001U,
    /// PC-relative signed 16-bit
    rel16         = 0b10000010U,
    /// PC-relative signed 32-bit
    rel32         = 0b10000100U,
    /// PC-relative signed 64-bit
    rel64         = 0b10001000U,
  };

  constexpr inline u8_t reg_bit_v         = 0b00010000U;
  constexpr inline u8_t mem_bit_v         = 0b00100000U;
  constexpr inline u8_t imm_bit_v         = 0b01000000U;
  constexpr inline u8_t rel_bit_v         = 0b10000000U;
  constexpr inline u8_t reg_or_mem_mask_v = 0b00110000U;
  constexpr inline u8_t rel_or_imm_mask_v = 0b11000000U;
  constexpr inline u8_t rel_or_mem_mask_v = 0b10100000U;

  enum operand_imm_t { operand_imm };
  enum operand_mem_t { operand_mem };
  enum operand_rel_t { operand_rel };

  class operand final {
  public:
    J_A(ND) inline operand() noexcept = default;

    J_A(ND,HIDDEN) inline operand(reg reg) noexcept
      : m_type(operand_type::reg),
        m_reg(reg)
    { J_ASSERT_NOT_NULL(reg); }

    J_A(AI,ND,HIDDEN) inline operand(operand_imm_t, operand_type t, i64_t imm) noexcept
      : m_type(t),
        m_imm_or_disp(imm)
    { }

    J_A(AI,ND,HIDDEN) inline operand(operand_imm_t, mem_width w, i64_t imm) noexcept
      : m_type((operand_type)(imm_bit_v | mem_width_mask(w))),
        m_imm_or_disp(imm)
    { }

    J_A(ND) inline operand(operand_mem_t, mem_width w, reg base, reg index, mem_scale scale, i32_t disp) noexcept
      : operand((operand_type)(mem_bit_v | mem_width_mask(w)), base, index, scale, disp)
    { }

    J_A(ND) inline operand(operand_rel_t, mem_width w, i32_t disp) noexcept
      : m_type((operand_type)(rel_bit_v | mem_width_mask(w))),
        m_imm_or_disp(disp)
    { }

    operand(operand_imm_t, mem_width w, i64_t addend, struct reloc_source source) noexcept;

    operand(operand_rel_t, mem_width w, i64_t addend, struct reloc_source source) noexcept;

    operand(operand_type type,
            reg base, reg index,
            mem_scale scale, i32_t disp) noexcept;


    J_A(AI,NODISC,HIDDEN,ND) inline explicit operator bool() const noexcept
    { return m_type != operand_type::none; }

    J_A(AI,NODISC,HIDDEN,ND) inline bool operator!() const noexcept
    { return m_type == operand_type::none; }

    J_A(AI,NODISC,HIDDEN,ND) inline bool empty() const noexcept
    { return m_type == operand_type::none; }

    J_A(AI,NODISC,HIDDEN,ND) inline operand_type type() const noexcept { return m_type; }

    [[nodiscard]] mem_width width() const noexcept;

    [[nodiscard]] inline union reg reg() const noexcept {
      J_ASSERT(m_type == operand_type::reg);
      return m_reg;
    }

    J_A(AI,NODISC,HIDDEN,ND) inline union reg base() const noexcept {
      return m_reg;
    }

    [[nodiscard]] inline union reg reg_or_base() const noexcept {
      J_ASSERT((u8_t)m_type & reg_or_mem_mask_v);
      return m_reg;
    }

    inline union reg index() const noexcept {
      J_ASSERT((u8_t)m_type & mem_bit_v);
      return m_index_reg;
    }

    [[nodiscard]] inline mem_scale scale() const noexcept {
      J_ASSERT((u8_t)m_type & mem_bit_v);
      return m_scale;
    }

    [[nodiscard]] inline i32_t disp() const noexcept {
      J_ASSERT((u8_t)m_type & rel_or_mem_mask_v);
      return m_imm_or_disp;
    }

    inline void set_disp(i32_t d) noexcept {
      J_ASSERT((u8_t)m_type & rel_or_mem_mask_v);
      m_imm_or_disp = d;
    }

    J_A(AI,ND,HIDDEN,NODISC) inline bool is_reg() const noexcept
    { return m_type == operand_type::reg; }

    J_A(AI,ND,HIDDEN,NODISC) inline bool is_gpr() const noexcept
    { return m_type == operand_type::reg && m_reg.is_gpr(); }

    J_A(AI,ND,HIDDEN,NODISC) inline bool is_fp_reg() const noexcept
    { return m_type == operand_type::reg && m_reg.is_fp(); }

    J_A(AI,ND,HIDDEN,NODISC) inline bool is_imm() const noexcept
    { return (u8_t)m_type & imm_bit_v; }

    J_A(AI,ND,HIDDEN,NODISC) inline bool is_mem() const noexcept
    { return (u8_t)m_type & mem_bit_v; }

    J_A(AI,ND,HIDDEN,NODISC) inline bool is_rel() const noexcept
    { return (u8_t)m_type & rel_bit_v; }

    J_A(AI,ND,HIDDEN,NODISC) inline bool is_reloc() const noexcept
    { return m_reloc_source.source_type != reloc_source_type::none; }

    [[nodiscard]] inline i64_t imm() const noexcept {
      J_ASSERT((u8_t)m_type & rel_or_imm_mask_v);
      return m_imm_or_disp;
    }

    [[nodiscard]] inline i64_t imm8() const noexcept {
      J_ASSUME(m_type == operand_type::imm8);
      return m_imm_or_disp;
    }

    [[nodiscard]] inline i64_t imm16() const noexcept {
      J_ASSUME(m_type == operand_type::imm16);
      return m_imm_or_disp;
    }

    [[nodiscard]] inline i64_t imm32() const noexcept {
      J_ASSUME(m_type == operand_type::imm32);
      return m_imm_or_disp;
    }

    [[nodiscard]] inline i64_t imm64() const noexcept {
      J_ASSUME(m_type == operand_type::imm64);
      return m_imm_or_disp;
    }

    [[nodiscard]] operand with_width(mem_width width) const noexcept;

    J_INLINE_GETTER struct reloc_source reloc_source() const noexcept {
      return m_reloc_source;
    }

    [[nodiscard]] bool operator==(const operand & rhs) const noexcept = default;

    J_A(AI,ND,HIDDEN,NODISC) inline bool operator==(union reg rhs) const noexcept {
      return m_type == operand_type::reg && m_reg == rhs;
    }

    J_A(AI,ND,HIDDEN,NODISC) inline bool is_phys(union reg rhs) const noexcept {
      return m_type == operand_type::reg && m_reg.phys() == rhs.phys();
    }
  protected:
    operand_type m_type = operand_type::none;
    union reg m_reg{};
    union reg m_index_reg{};
    mem_scale m_scale = mem_scale::byte;
    struct reloc_source m_reloc_source;
    i64_t m_imm_or_disp = 0U;
  };

  J_A(AI,ND,NODISC) inline operand memop(mem_width w, reg r, i32_t disp = 0) noexcept
  { return operand(operand_mem, w, r, {}, mem_scale::byte, disp); }

  J_A(AI,ND,NODISC) inline operand memop(mem_width w, reg r, reg index, mem_scale scale = mem_scale::byte, i32_t disp = 0) noexcept
  { return operand(operand_mem, w, r, index, scale, disp); }

  J_A(AI,ND,NODISC) inline operand mem8(reg r, i32_t disp = 0) noexcept
  { return operand(operand_mem, mem_width::byte, r, {}, mem_scale::byte, disp); }

  J_A(AI,ND,NODISC) inline operand mem8(reg r, reg index, mem_scale scale = mem_scale::byte, i32_t disp = 0) noexcept
  { return operand(operand_mem, mem_width::byte, r, index, scale, disp); }

  J_A(AI,ND,NODISC) inline operand mem16(reg r, i32_t disp = 0) noexcept
  { return operand(operand_mem, mem_width::word, r, {}, mem_scale::byte, disp); }

  J_A(AI,ND,NODISC) inline operand mem16(reg r, reg index, mem_scale scale = mem_scale::byte, i32_t disp = 0) noexcept
  { return operand(operand_mem, mem_width::word, r, index, scale, disp); }

  J_A(AI,ND,NODISC) inline operand mem32(reg r, i32_t disp = 0) noexcept
  { return operand(operand_mem, mem_width::dword, r, {}, mem_scale::byte, disp); }

  J_A(AI,ND,NODISC) inline operand mem32(reg r, reg index, mem_scale scale = mem_scale::byte, i32_t disp = 0) noexcept
  { return operand(operand_mem, mem_width::dword, r, index, scale, disp); }

  J_A(AI,ND,NODISC) inline operand mem64(reg r, i32_t disp = 0) noexcept
  { return operand(operand_mem, mem_width::qword, r, {}, mem_scale::byte, disp); }

  J_A(AI,ND,NODISC) inline operand mem64(reg r, reg index, mem_scale scale = mem_scale::byte, i32_t disp = 0) noexcept
  { return operand(operand_mem, mem_width::qword, r, index, scale, disp); }

  J_A(AI,ND,NODISC) inline operand imm(mem_width w, i64_t imm) noexcept
  { return operand(operand_imm, w, imm); }

  J_A(AI,ND,NODISC) inline operand imm8(i8_t imm) noexcept
  { return operand(operand_imm, operand_type::imm8, imm); }

  J_A(AI,ND,NODISC) inline operand imm16(i16_t imm) noexcept
  { return operand(operand_imm, operand_type::imm16, imm); }

  J_A(AI,ND,NODISC) inline operand imm32(i32_t imm) noexcept
  { return operand(operand_imm, operand_type::imm32, imm); }

  J_A(AI,ND,NODISC) inline operand imm64(i64_t imm) noexcept
  { return operand(operand_imm, operand_type::imm64, imm); }

  [[nodiscard]] inline operand imm(i64_t imm) noexcept {
    mem_width w = mem_width::qword;
    if (bits::fits_i8(imm)) {
      w = mem_width::byte;
    } else if (bits::fits_i16(imm)) {
      w = mem_width::word;
    } else if (bits::fits_i32(imm)) {
      w = mem_width::dword;
    }
    return operand(operand_imm, w, imm);
  }

  [[nodiscard]] inline operand imm(mem_width w, reloc_source source, i64_t addend = 0) noexcept
  { return operand(operand_imm, w, addend, source); }

  [[nodiscard]] inline operand imm8(reloc_source source, i8_t addend = 0) noexcept
  { return operand(operand_imm, mem_width::byte, addend, source); }

  [[nodiscard]] inline operand imm16(reloc_source source, i16_t addend = 0) noexcept
  { return operand(operand_imm, mem_width::word, addend, source); }

  [[nodiscard]] inline operand imm32(reloc_source source, i32_t addend = 0) noexcept
  { return operand(operand_imm, mem_width::dword, addend, source); }

  [[nodiscard]] inline operand imm64(reloc_source source, i64_t addend = 0) noexcept
  { return operand(operand_imm, mem_width::qword, addend, source); }

  J_A(AI,ND,HIDDEN,NODISC) inline operand rel(mem_width w, i32_t disp) noexcept
  { return operand(operand_rel, w, disp); }

  J_A(AI,ND,HIDDEN,NODISC) inline operand rel8(i32_t disp) noexcept
  { return operand(operand_rel, mem_width::byte, disp); }

  J_A(AI,ND,HIDDEN,NODISC) inline operand rel16(i32_t disp) noexcept
  { return operand(operand_rel, mem_width::word, disp); }

  J_A(AI,ND,HIDDEN,NODISC) inline operand rel32(i32_t disp) noexcept
  { return operand(operand_rel, mem_width::dword, disp); }

  J_A(AI,ND,HIDDEN,NODISC) inline operand rel64(i32_t disp) noexcept
  { return operand(operand_rel, mem_width::qword, disp); }

  [[nodiscard]] inline operand rel(mem_width w, reloc_source source, i32_t addend = 0U) noexcept
  { return operand(operand_rel, w, addend, source); }

  [[nodiscard]] inline operand rel8(reloc_source source, i8_t addend = 0U) noexcept
  { return operand(operand_rel, mem_width::byte, addend, source); }

  [[nodiscard]] inline operand rel16(reloc_source source, i16_t addend = 0U) noexcept
  { return operand(operand_rel, mem_width::word, addend, source); }

  [[nodiscard]] inline operand rel32(reloc_source source, i32_t addend = 0U) noexcept
  { return operand(operand_rel, mem_width::dword, addend, source); }

  [[nodiscard]] inline operand rel64(reloc_source source, i64_t addend = 0U) noexcept
  { return operand(operand_rel, mem_width::qword, addend, source); }
}
