#pragma once

#include "lisp/assembly/amd64/instruction.hpp"

namespace j::lisp::assembly::amd64 {
  J_RETURNS_NONNULL char * emit_no_imm(
    char * J_NOT_NULL ptr,
    const instruction_encoding * J_NOT_NULL enc,
    const operand * operands,
    void ** disp_addr = nullptr);

  J_RETURNS_NONNULL char * emit(char * J_NOT_NULL ptr,
                                const instruction_encoding * J_NOT_NULL enc,
                                const operand * operands,
                                void ** disp_addr = nullptr,
                                void ** imm_addr = nullptr);
}
