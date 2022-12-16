#pragma once

#include "lisp/assembly/operand.hpp"
#include "lisp/assembly/amd64/registers.hpp"

namespace j::lisp::assembly::amd64 {
  J_INLINE_GETTER_NONNULL constexpr char * write_opcode(char * J_NOT_NULL ptr, u8_t b0) noexcept {
    *ptr++ = b0;
    return ptr;
  }

  J_INLINE_GETTER_NONNULL constexpr char * write_opcode(char * J_NOT_NULL ptr, u8_t b0, u8_t b1) noexcept {
    *ptr++ = b0;
    *ptr++ = b1;
    return ptr;
  }

  J_INLINE_GETTER_NONNULL constexpr char * write_opcode(char * J_NOT_NULL ptr, u8_t b0, u8_t b1, u8_t b2) noexcept {
    *ptr++ = b0;
    *ptr++ = b1;
    *ptr++ = b2;
    return ptr;
  }

  J_RETURNS_NONNULL constexpr char * write_opcode_plus_r(char * J_AA(NN,NOALIAS) ptr, u8_t b0, const operand & J_AA(NOALIAS) op) noexcept {
    J_ASSERT(!(b0 & 0b111U));
    *ptr++ = b0 | (op.reg().index() & 7U);
    return ptr;
  }

  J_RETURNS_NONNULL constexpr char * write_opcode_plus_r(char * J_AA(NN,NOALIAS) ptr, u8_t b0, u8_t b1, const operand & J_AA(NOALIAS) op) noexcept {
    J_ASSERT(!(b1 & 0b111U));
    *ptr++ = b0;
    *ptr++ = b1 | (op.reg().index() & 7U);
    return ptr;
  }

  J_RETURNS_NONNULL constexpr char * write_opcode_plus_r(char * J_AA(NN,NOALIAS) ptr, u8_t b0, u8_t b1, u8_t b2, const operand & J_AA(NOALIAS) op) noexcept {
    J_ASSERT(!(b2 & 0b111U));
    *ptr++ = b0;
    *ptr++ = b1;
    *ptr++ = b2 | (op.reg().index() & 7U);
    return ptr;
  }
}
