#pragma once
// GENERATED HEADER

#include <lisp/assembly/amd64/instruction.hpp>

namespace j::lisp::assembly::amd64::inst {
  constexpr inline instruction<3> lea = {
    .name      = "lea",
    .encodings = {
      {
        .mask = operand_mask(operand_type_mask::r32, operand_type_mask::mem),
        .operand_format = operand_format::rm,
        .opcode_format = opcode_format::plain_1,
        .opcode = {0x8DU, 0x00U, 0x00U, 0x00U},
        .name = "8D /r",
      },
      {
        .mask = operand_mask(operand_type_mask::r16, operand_type_mask::mem),
        .operand_format = operand_format::rm,
        .opcode_format = opcode_format::plain_2,
        .opcode = {0x67U, 0x8DU, 0x00U, 0x00U},
        .name = "67 8D /r",
      },
      {
        .mask = operand_mask(operand_type_mask::r64, operand_type_mask::mem),
        .has_rex_w = true,
        .operand_format = operand_format::rm,
        .opcode_format = opcode_format::plain_1,
        .opcode = {0x8DU, 0x00U, 0x00U, 0x00U},
        .name = "REX.W 8D /r",
      },
      instruction_encoding{},
    },
  };
}
