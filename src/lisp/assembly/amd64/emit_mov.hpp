#pragma once

#include "lisp/assembly/amd64/emitter.hpp"
#include "lisp/assembly/amd64/inst/generated/mov.hpp"
#include "lisp/assembly/amd64/inst/generated/binop.hpp"
#include "lisp/assembly/functions/function_builder.hpp"
#include "exceptions/assert.hpp"

namespace j::lisp::assembly::amd64 {
  inline void emit_mov(function_builder & buf, operand to, operand from) noexcept {
    namespace i = inst;
    const bool to_is_reg = to.is_reg();
    if (from.is_imm()) {
      i64_t imm = from.imm();
      if (to_is_reg && !imm && !from.is_reloc()) {
        emit(buf, i::bxor, to.with_width(mem_width::dword), to.with_width(mem_width::dword));
        return;
      }
      u64_t immu = (u64_t)imm;
      if (to_is_reg && !from.is_reloc() && immu <= U32_MAX) {
        emit(buf, i::mov, to.with_width(mem_width::dword), from.with_width(mem_width::dword));
        return;
      } else if (to.width() ==mem_width::qword  &&  to_is_reg && !from.is_reloc() && imm >= I32_MIN && imm <= I32_MAX) {
        emit(buf, i::mov, to.with_width(mem_width::qword), from.with_width(mem_width::dword));
        return;
      } else if (to.is_mem() && to.width() == mem_width::qword) {
        from = from.with_width(mem_width::dword);
      } else if (from.width() != to.width()) {
        from = from.with_width(to.width());
      }
      emit(buf, i::mov, to, from);
      return;
    }
    if (to_is_reg && from.is_reg() && to.reg() == from.reg()) {
      return;
    }
    if (to.width() == from.width() || to.is_reloc()) {
      emit(buf, i::mov, to, from);
      return;
    }
    if (to_is_reg && to.width() == mem_width::qword && from.width() <= mem_width::dword) {
      emit(buf, i::mov, to.reg().with_width(mem_width::dword), from.with_width(mem_width::dword));
      return;
    }
    if (from.is_reg()) {
      emit(buf, i::mov, to, from.reg().with_width(to.width()));
      return;
    }

    J_ASSERT_FAIL("Unsupported mov case: mov {} {}", to, from);
  }
}
