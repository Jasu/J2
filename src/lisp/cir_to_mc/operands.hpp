#pragma once

#include "lisp/cir/ops/ops.hpp"
#include "lisp/assembly/compiler.hpp"

namespace j::lisp::cir_to_mc {
  namespace {
    namespace o = cir::ops::defs;
    namespace a = assembly;

    [[nodiscard]] inline a::reg to_asm_reg(cir::loc loc, mem_width width = mem_width::qword) noexcept {
      return a::make_reg(loc.as_phys_reg().index, width);
    }

    [[nodiscard]] [[maybe_unused]] inline a::reg to_asm_reg(const cir::input & oper, mem_width width = mem_width::qword) noexcept {
      J_ASSERT(oper.is_op_result());
      return a::make_reg(oper.op_result_data.use.loc_out.as_phys_reg().index, width);
    }

    template<typename T>
    J_A(AI,ND,NODISC) a::reg to_asm_reg64(const T & in) noexcept {
      return to_asm_reg(in, mem_width::qword);
    }

    template<typename T>
    J_A(AI,ND,NODISC) a::reg to_asm_reg32(const T & in) noexcept {
      return to_asm_reg(in, mem_width::dword);
    }

    template<typename T>
    J_A(AI,ND,NODISC) a::reg to_asm_reg8(const T & in) noexcept {
      return to_asm_reg(in, mem_width::byte);
    }

    [[nodiscard]] inline a::reloc_source to_asm_reloc_source(const cir::input & oper) noexcept {
      J_ASSERT(oper.is_reloc());

      a::reloc_source_type type;

      functions::calling_convention cc = functions::calling_convention::none;
      auto & d = oper.reloc_data;
      switch (d.reloc_type) {
      case cir::reloc_type::none:
        J_FAIL("Input had an empty reloc");
      case cir::reloc_type::constant_addr:
        type = a::reloc_source_type::constant_addr;
        break;
      case cir::reloc_type::var_value_untagged:
        type = a::reloc_source_type::var_value_untagged;
        break;
      case cir::reloc_type::var_value_tagged:
        type = a::reloc_source_type::var_value_tagged;
        break;
      case cir::reloc_type::var_addr:
        type = a::reloc_source_type::var_addr;
        break;
      case cir::reloc_type::fn_addr:
        cc = d.calling_convention;
        type = a::reloc_source_type::fn_addr;
        break;
      }

      return a::reloc_source{type, cc, { .constant_offset = d.constant_offset }};
    }

    [[nodiscard]] [[maybe_unused]] inline a::operand to_asm_mem_operand(const cir::input & oper) noexcept {
      switch (oper.type) {
      case cir::input_type::none:
      case cir::input_type::constant:
        J_FAIL("Invalid mem operand");
      case cir::input_type::op_result:
        return a::mem64(to_asm_reg64(oper.op_result_data.use.loc_in));
      case cir::input_type::reloc:
        return a::rel64(to_asm_reloc_source(oper), oper.reloc_data.addend);
      case cir::input_type::mem:
        break;
      }

      auto & m = oper.mem_data;
      J_ASSERT(m.base.is_op_result());
      a::reg base = to_asm_reg64(m.base.op_result_data.use.loc_in);
      i32_t displacement = m.displacement;
      if (m.index.is_op_result()) {
        a::reg index = to_asm_reg64(m.index.op_result_data.use.loc_in);
        return a::memop(m.width, base, index, m.scale, displacement);
      } else if (m.index) {
        displacement += m.index.const_data * scale_to_bytes(m.scale, m.width);
      }
      return a::memop(m.width, base, displacement);
    }

    [[nodiscard]] [[maybe_unused]] inline a::operand to_asm_operand(a::compiler J_NOT_NULL * c, cir::loc loc, mem_width type = mem_width::qword) noexcept {
      if (loc.type() == cir::loc_type::spill_slot) {
        return c->spill_slot_operand(loc.spill_slot_index());
      }
      return to_asm_reg(loc, type);
    }

    [[nodiscard]] [[maybe_unused]] inline a::operand to_asm_operand(a::compiler J_NOT_NULL * c, const cir::input & oper, mem_width type = mem_width::qword) noexcept {
      switch (oper.type) {
      case cir::input_type::none:
        J_FAIL("Invalid operand");
      case cir::input_type::mem:
        return to_asm_mem_operand(oper);
      case cir::input_type::op_result:
        return to_asm_operand(c, oper.op_result_data.use.loc_in, type);
      case cir::input_type::constant: {
        i64_t cv = (i64_t)oper.const_data;
        if (cv >= -128 && cv < 128) {
          return a::imm8(cv);
        }
        J_ASSERT(cv >= I32_MIN && cv <= I32_MAX);
        return a::imm32(cv);
      }

      case cir::input_type::reloc:
        return a::imm64(to_asm_reloc_source(oper), oper.reloc_data.addend);
      }
    }
  }
}
