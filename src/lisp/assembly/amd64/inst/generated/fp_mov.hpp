#pragma once
// GENERATED HEADER

#include <lisp/assembly/amd64/instruction.hpp>

namespace j::lisp::assembly::amd64::inst {
  constexpr inline instruction<2> movd = {
    .name      = "movd",
    .encodings = {
      {
        .mask = operand_mask(operand_type_mask::rm32, operand_type_mask::xmm),
        .mandatory_prefix = mandatory_prefix::has_66,
        .operand_format = operand_format::mr,
        .opcode_format = opcode_format::plain_2,
        .opcode = {0x0FU, 0x7EU, 0x00U, 0x00U},
        .name = "66 0F 7E /r",
      },
      {
        .mask = operand_mask(operand_type_mask::xmm, operand_type_mask::rm32),
        .mandatory_prefix = mandatory_prefix::has_66,
        .operand_format = operand_format::rm,
        .opcode_format = opcode_format::plain_2,
        .opcode = {0x0FU, 0x6EU, 0x00U, 0x00U},
        .name = "66 0F 6E /r",
      },
      instruction_encoding{},
    },
  };
  constexpr inline instruction<4> movq = {
    .name      = "movq",
    .encodings = {
      {
        .mask = operand_mask(operand_type_mask::xmm, operand_type_mask::xmm_m64),
        .mandatory_prefix = mandatory_prefix::has_f3,
        .operand_format = operand_format::rm,
        .opcode_format = opcode_format::plain_2,
        .opcode = {0x0FU, 0x7EU, 0x00U, 0x00U},
        .name = "F3 0F 7E /r",
      },
      {
        .mask = operand_mask(operand_type_mask::xmm_m64, operand_type_mask::xmm),
        .mandatory_prefix = mandatory_prefix::has_66,
        .operand_format = operand_format::mr,
        .opcode_format = opcode_format::plain_2,
        .opcode = {0x0FU, 0xD6U, 0x00U, 0x00U},
        .name = "66 0F D6 /r",
      },
      {
        .mask = operand_mask(operand_type_mask::rm64, operand_type_mask::xmm),
        .mandatory_prefix = mandatory_prefix::has_66,
        .has_rex_w = true,
        .operand_format = operand_format::mr,
        .opcode_format = opcode_format::plain_2,
        .opcode = {0x0FU, 0x7EU, 0x00U, 0x00U},
        .name = "REX.W 66 0F 7E /r",
      },
      {
        .mask = operand_mask(operand_type_mask::xmm, operand_type_mask::rm64),
        .mandatory_prefix = mandatory_prefix::has_66,
        .has_rex_w = true,
        .operand_format = operand_format::rm,
        .opcode_format = opcode_format::plain_2,
        .opcode = {0x0FU, 0x6EU, 0x00U, 0x00U},
        .name = "REX.W 66 0F 6E /r",
      },
      instruction_encoding{},
    },
  };
}
