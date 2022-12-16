#pragma once
// GENERATED HEADER

#include <lisp/assembly/amd64/instruction.hpp>

namespace j::lisp::assembly::amd64::inst {
  constexpr inline instruction<4> pop = {
    .name      = "pop",
    .encodings = {
      {
        .mask = operand_mask(operand_type_mask::r64),
        .operand_format = operand_format::ri,
        .opcode_format = opcode_format::plus_reg_1,
        .opcode = {0x58U, 0x00U, 0x00U, 0x00U},
        .name = "58+r",
      },
      {
        .mask = operand_mask(operand_type_mask::r16),
        .mandatory_prefix = mandatory_prefix::has_66,
        .operand_format = operand_format::ri,
        .opcode_format = opcode_format::plus_reg_1,
        .opcode = {0x58U, 0x00U, 0x00U, 0x00U},
        .name = "66 58+r",
      },
      {
        .mask = operand_mask(operand_type_mask::rm64),
        .operand_format = operand_format::m,
        .opcode_format = opcode_format::plain_1,
        .modrm_forced_r = 0,
        .opcode = {0x8FU, 0x00U, 0x00U, 0x00U},
        .name = "8F /0",
      },
      {
        .mask = operand_mask(operand_type_mask::rm16),
        .mandatory_prefix = mandatory_prefix::has_66,
        .operand_format = operand_format::m,
        .opcode_format = opcode_format::plain_1,
        .modrm_forced_r = 0,
        .opcode = {0x8FU, 0x00U, 0x00U, 0x00U},
        .name = "66 8F /0",
      },
      instruction_encoding{},
    },
  };
}
