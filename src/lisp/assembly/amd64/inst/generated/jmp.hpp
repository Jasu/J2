#pragma once
// GENERATED HEADER

#include <lisp/assembly/amd64/instruction.hpp>

namespace j::lisp::assembly::amd64::inst {
  constexpr inline instruction<2> call = {
    .name      = "call",
    .encodings = {
      {
        .mask = operand_mask(operand_type_mask::rm64),
        .operand_format = operand_format::m,
        .opcode_format = opcode_format::plain_1,
        .modrm_forced_r = 2,
        .opcode = {0xFFU, 0x00U, 0x00U, 0x00U},
        .name = "FF /2",
      },
      {
        .mask = operand_mask(operand_type_mask::imm32),
        .operand_format = operand_format::none,
        .opcode_format = opcode_format::plain_1,
        .imm_format = imm_format::rel32,
        .opcode = {0xE8U, 0x00U, 0x00U, 0x00U},
        .name = "E8 cd",
      },
      instruction_encoding{},
    },
  };
  constexpr inline instruction<3> jmp = {
    .name      = "jmp",
    .encodings = {
      {
        .mask = operand_mask(operand_type_mask::rm64),
        .operand_format = operand_format::m,
        .opcode_format = opcode_format::plain_1,
        .modrm_forced_r = 4,
        .opcode = {0xFFU, 0x00U, 0x00U, 0x00U},
        .name = "FF /4",
      },
      {
        .mask = operand_mask(operand_type_mask::imm8),
        .operand_format = operand_format::none,
        .opcode_format = opcode_format::plain_1,
        .imm_format = imm_format::rel8,
        .opcode = {0xEBU, 0x00U, 0x00U, 0x00U},
        .name = "EB cb",
      },
      {
        .mask = operand_mask(operand_type_mask::imm32),
        .operand_format = operand_format::none,
        .opcode_format = opcode_format::plain_1,
        .imm_format = imm_format::rel32,
        .opcode = {0xE9U, 0x00U, 0x00U, 0x00U},
        .name = "E9 cd",
      },
      instruction_encoding{},
    },
  };
}
