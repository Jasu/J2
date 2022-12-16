#pragma once
// GENERATED HEADER

#include <lisp/assembly/amd64/instruction.hpp>

namespace j::lisp::assembly::amd64::inst {
  constexpr inline instruction<7> push = {
    .name      = "push",
    .encodings = {
      {
        .mask = operand_mask(operand_type_mask::r64),
        .operand_format = operand_format::ri,
        .opcode_format = opcode_format::plus_reg_1,
        .opcode = {0x50U, 0x00U, 0x00U, 0x00U},
        .name = "50+r",
      },
      {
        .mask = operand_mask(operand_type_mask::r16),
        .mandatory_prefix = mandatory_prefix::has_66,
        .operand_format = operand_format::ri,
        .opcode_format = opcode_format::plus_reg_1,
        .opcode = {0x50U, 0x00U, 0x00U, 0x00U},
        .name = "66 50+r",
      },
      {
        .mask = operand_mask(operand_type_mask::rm64),
        .operand_format = operand_format::m,
        .opcode_format = opcode_format::plain_1,
        .modrm_forced_r = 6,
        .opcode = {0xFFU, 0x00U, 0x00U, 0x00U},
        .name = "FF /6",
      },
      {
        .mask = operand_mask(operand_type_mask::imm8),
        .operand_format = operand_format::none,
        .opcode_format = opcode_format::plain_1,
        .imm_format = imm_format::imm8,
        .opcode = {0x6AU, 0x00U, 0x00U, 0x00U},
        .name = "6A ib",
      },
      {
        .mask = operand_mask(operand_type_mask::imm32),
        .operand_format = operand_format::none,
        .opcode_format = opcode_format::plain_1,
        .imm_format = imm_format::imm32,
        .opcode = {0x68U, 0x00U, 0x00U, 0x00U},
        .name = "68 id",
      },
      {
        .mask = operand_mask(operand_type_mask::imm16),
        .mandatory_prefix = mandatory_prefix::has_66,
        .operand_format = operand_format::none,
        .opcode_format = opcode_format::plain_1,
        .imm_format = imm_format::imm16,
        .opcode = {0x68U, 0x00U, 0x00U, 0x00U},
        .name = "66 68 iw",
      },
      {
        .mask = operand_mask(operand_type_mask::rm16),
        .mandatory_prefix = mandatory_prefix::has_66,
        .operand_format = operand_format::m,
        .opcode_format = opcode_format::plain_1,
        .modrm_forced_r = 6,
        .opcode = {0xFFU, 0x00U, 0x00U, 0x00U},
        .name = "66 FF /6",
      },
      instruction_encoding{},
    },
  };
}
