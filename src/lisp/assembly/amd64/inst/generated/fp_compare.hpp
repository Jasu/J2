#pragma once
// GENERATED HEADER

#include <lisp/assembly/amd64/instruction.hpp>

namespace j::lisp::assembly::amd64::inst {
  constexpr inline instruction<1> ucomisd = {
    .name      = "ucomisd",
    .encodings = {
      {
        .mask = operand_mask(operand_type_mask::xmm, operand_type_mask::xmm_m64),
        .mandatory_prefix = mandatory_prefix::has_66,
        .operand_format = operand_format::rm,
        .opcode_format = opcode_format::plain_2,
        .opcode = {0x0FU, 0x2EU, 0x00U, 0x00U},
        .name = "66 0F 2E /r",
      },
      instruction_encoding{},
    },
  };
  constexpr inline instruction<1> ucomiss = {
    .name      = "ucomiss",
    .encodings = {
      {
        .mask = operand_mask(operand_type_mask::xmm, operand_type_mask::xmm_m32),
        .operand_format = operand_format::rm,
        .opcode_format = opcode_format::plain_2,
        .opcode = {0x0FU, 0x2EU, 0x00U, 0x00U},
        .name = "0F 2E /r",
      },
      instruction_encoding{},
    },
  };
}
