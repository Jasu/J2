#include "lisp/assembly/operand.hpp"

namespace j::lisp::assembly {
  [[nodiscard]] mem_width operand::width() const noexcept {
    switch (m_type) {
    case operand_type::none:
      return mem_width::none;
    case operand_type::reg:
      return m_reg.width();
    case operand_type::mem8:
    case operand_type::imm8:
    case operand_type::rel8:
      return mem_width::byte;
    case operand_type::mem16:
    case operand_type::imm16:
    case operand_type::rel16:
      return mem_width::word;
    case operand_type::imm32:
    case operand_type::mem32:
    case operand_type::rel32:
      return mem_width::dword;
    case operand_type::rel64:
    case operand_type::mem64:
    case operand_type::imm64:
      return mem_width::qword;
    }
  }

  operand::operand(operand_imm_t, mem_width w, i64_t addend, struct reloc_source source) noexcept
      : m_type((operand_type)(imm_bit_v | mem_width_mask(w))),
        m_reloc_source(source),
        m_imm_or_disp(addend)
    { }

  operand::operand(operand_rel_t, mem_width w, i64_t addend, struct reloc_source source) noexcept
      : m_type((operand_type)(rel_bit_v | mem_width_mask(w))),
        m_reloc_source(source),
        m_imm_or_disp(addend)
    { }

  operand::operand(operand_type type,
                   union reg base, union reg index,
                   mem_scale scale, i32_t disp) noexcept
    : m_type(type),
      m_reg(base),
      m_index_reg(index),
      m_scale(scale),
      m_imm_or_disp(disp)
  {
    J_ASSERT(!base || base.is_gpr());
    J_ASSERT(!index || index.is_gpr());
  }

  [[nodiscard]] operand operand::with_width(mem_width width) const noexcept {
    J_ASSUME(m_type != operand_type::none);
    if (m_type == operand_type::reg) {
      return operand(m_reg.with_width(width));
    }
    if ((u8_t)m_type & mem_bit_v) {
      return operand((operand_type)(mem_bit_v | mem_width_mask(width)),
                     m_reg,
                     m_index_reg,
                     m_scale,
                     m_imm_or_disp);
    }
    if ((u8_t)m_type & imm_bit_v) {
      return operand(operand_imm, width, m_imm_or_disp);
    }
    J_ASSERT_FAIL("Unsupported type for set_width");
  }

}
