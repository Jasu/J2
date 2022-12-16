#pragma once
// GENERATED HEADER

#include <lisp/assembly/amd64/instruction.hpp>

namespace j::lisp::assembly::amd64::inst {
  constexpr inline instruction<6> bt = {
    .name      = "bt",
    .encodings = {
      {
        .mask = operand_mask(operand_type_mask::rm32, operand_type_mask::imm8),
        .operand_format = operand_format::m,
        .opcode_format = opcode_format::plain_2,
        .modrm_forced_r = 4,
        .imm_format = imm_format::imm8,
        .opcode = {0x0FU, 0xBAU, 0x00U, 0x00U},
        .name = "0F BA /4 ib",
      },
      {
        .mask = operand_mask(operand_type_mask::rm32, operand_type_mask::r32),
        .operand_format = operand_format::mr,
        .opcode_format = opcode_format::plain_2,
        .opcode = {0x0FU, 0xA3U, 0x00U, 0x00U},
        .name = "0F A3 /r",
      },
      {
        .mask = operand_mask(operand_type_mask::rm64, operand_type_mask::imm8),
        .has_rex_w = true,
        .operand_format = operand_format::m,
        .opcode_format = opcode_format::plain_2,
        .modrm_forced_r = 4,
        .imm_format = imm_format::imm8,
        .opcode = {0x0FU, 0xBAU, 0x00U, 0x00U},
        .name = "REX.W 0F BA /4 ib",
      },
      {
        .mask = operand_mask(operand_type_mask::rm64, operand_type_mask::r64),
        .has_rex_w = true,
        .operand_format = operand_format::mr,
        .opcode_format = opcode_format::plain_2,
        .opcode = {0x0FU, 0xA3U, 0x00U, 0x00U},
        .name = "REX.W 0F A3 /r",
      },
      {
        .mask = operand_mask(operand_type_mask::rm16, operand_type_mask::imm8),
        .mandatory_prefix = mandatory_prefix::has_66,
        .operand_format = operand_format::m,
        .opcode_format = opcode_format::plain_2,
        .modrm_forced_r = 4,
        .imm_format = imm_format::imm8,
        .opcode = {0x0FU, 0xBAU, 0x00U, 0x00U},
        .name = "66 0F BA /4 ib",
      },
      {
        .mask = operand_mask(operand_type_mask::rm16, operand_type_mask::r16),
        .mandatory_prefix = mandatory_prefix::has_66,
        .operand_format = operand_format::mr,
        .opcode_format = opcode_format::plain_2,
        .opcode = {0x0FU, 0xA3U, 0x00U, 0x00U},
        .name = "66 0F A3 /r",
      },
      instruction_encoding{},
    },
  };
}
