#pragma once
// GENERATED HEADER

#include <lisp/assembly/amd64/instruction.hpp>

namespace j::lisp::assembly::amd64::inst {
  constexpr inline instruction<19> cmp = {
    .name      = "cmp",
    .encodings = {
      {
        .mask = operand_mask(special_case_eax, operand_type_mask::imm32),
        .operand_format = operand_format::implicit,
        .opcode_format = opcode_format::plain_1,
        .imm_format = imm_format::imm32,
        .opcode = {0x3DU, 0x00U, 0x00U, 0x00U},
        .name = "3D id",
      },
      {
        .mask = operand_mask(operand_type_mask::rm32, operand_type_mask::imm32),
        .operand_format = operand_format::m,
        .opcode_format = opcode_format::plain_1,
        .modrm_forced_r = 7,
        .imm_format = imm_format::imm32,
        .opcode = {0x81U, 0x00U, 0x00U, 0x00U},
        .name = "81 /7 id",
      },
      {
        .mask = operand_mask(operand_type_mask::rm32, operand_type_mask::imm8),
        .operand_format = operand_format::m,
        .opcode_format = opcode_format::plain_1,
        .modrm_forced_r = 7,
        .imm_format = imm_format::imm8,
        .opcode = {0x83U, 0x00U, 0x00U, 0x00U},
        .name = "83 /7 ib",
      },
      {
        .mask = operand_mask(operand_type_mask::rm32, operand_type_mask::r32),
        .operand_format = operand_format::mr,
        .opcode_format = opcode_format::plain_1,
        .opcode = {0x39U, 0x00U, 0x00U, 0x00U},
        .name = "39 /r",
      },
      {
        .mask = operand_mask(operand_type_mask::r32, operand_type_mask::rm32),
        .operand_format = operand_format::rm,
        .opcode_format = opcode_format::plain_1,
        .opcode = {0x3BU, 0x00U, 0x00U, 0x00U},
        .name = "3B /r",
      },
      {
        .mask = operand_mask(special_case_al, operand_type_mask::imm8),
        .operand_format = operand_format::implicit,
        .opcode_format = opcode_format::plain_1,
        .imm_format = imm_format::imm8,
        .opcode = {0x3CU, 0x00U, 0x00U, 0x00U},
        .name = "3C ib",
      },
      {
        .mask = operand_mask(operand_type_mask::rm8, operand_type_mask::imm8),
        .operand_format = operand_format::m,
        .opcode_format = opcode_format::plain_1,
        .modrm_forced_r = 7,
        .imm_format = imm_format::imm8,
        .opcode = {0x80U, 0x00U, 0x00U, 0x00U},
        .name = "80 /7 ib",
      },
      {
        .mask = operand_mask(operand_type_mask::rm8, operand_type_mask::r8),
        .operand_format = operand_format::mr,
        .opcode_format = opcode_format::plain_1,
        .opcode = {0x38U, 0x00U, 0x00U, 0x00U},
        .name = "38 /r",
      },
      {
        .mask = operand_mask(operand_type_mask::r8, operand_type_mask::rm8),
        .operand_format = operand_format::rm,
        .opcode_format = opcode_format::plain_1,
        .opcode = {0x3AU, 0x00U, 0x00U, 0x00U},
        .name = "3A /r",
      },
      {
        .mask = operand_mask(special_case_rax, operand_type_mask::imm32),
        .has_rex_w = true,
        .operand_format = operand_format::implicit,
        .opcode_format = opcode_format::plain_1,
        .imm_format = imm_format::imm32,
        .opcode = {0x3DU, 0x00U, 0x00U, 0x00U},
        .name = "REX.W 3D id",
      },
      {
        .mask = operand_mask(operand_type_mask::rm64, operand_type_mask::imm32),
        .has_rex_w = true,
        .operand_format = operand_format::m,
        .opcode_format = opcode_format::plain_1,
        .modrm_forced_r = 7,
        .imm_format = imm_format::imm32,
        .opcode = {0x81U, 0x00U, 0x00U, 0x00U},
        .name = "REX.W 81 /7 id",
      },
      {
        .mask = operand_mask(operand_type_mask::rm64, operand_type_mask::imm8),
        .has_rex_w = true,
        .operand_format = operand_format::m,
        .opcode_format = opcode_format::plain_1,
        .modrm_forced_r = 7,
        .imm_format = imm_format::imm8,
        .opcode = {0x83U, 0x00U, 0x00U, 0x00U},
        .name = "REX.W 83 /7 ib",
      },
      {
        .mask = operand_mask(operand_type_mask::rm64, operand_type_mask::r64),
        .has_rex_w = true,
        .operand_format = operand_format::mr,
        .opcode_format = opcode_format::plain_1,
        .opcode = {0x39U, 0x00U, 0x00U, 0x00U},
        .name = "REX.W 39 /r",
      },
      {
        .mask = operand_mask(operand_type_mask::r64, operand_type_mask::rm64),
        .has_rex_w = true,
        .operand_format = operand_format::rm,
        .opcode_format = opcode_format::plain_1,
        .opcode = {0x3BU, 0x00U, 0x00U, 0x00U},
        .name = "REX.W 3B /r",
      },
      {
        .mask = operand_mask(special_case_ax, operand_type_mask::imm16),
        .mandatory_prefix = mandatory_prefix::has_66,
        .operand_format = operand_format::implicit,
        .opcode_format = opcode_format::plain_1,
        .imm_format = imm_format::imm16,
        .opcode = {0x3DU, 0x00U, 0x00U, 0x00U},
        .name = "66 3D iw",
      },
      {
        .mask = operand_mask(operand_type_mask::rm16, operand_type_mask::imm16),
        .mandatory_prefix = mandatory_prefix::has_66,
        .operand_format = operand_format::m,
        .opcode_format = opcode_format::plain_1,
        .modrm_forced_r = 7,
        .imm_format = imm_format::imm16,
        .opcode = {0x81U, 0x00U, 0x00U, 0x00U},
        .name = "66 81 /7 iw",
      },
      {
        .mask = operand_mask(operand_type_mask::rm16, operand_type_mask::imm8),
        .mandatory_prefix = mandatory_prefix::has_66,
        .operand_format = operand_format::m,
        .opcode_format = opcode_format::plain_1,
        .modrm_forced_r = 7,
        .imm_format = imm_format::imm8,
        .opcode = {0x83U, 0x00U, 0x00U, 0x00U},
        .name = "66 83 /7 ib",
      },
      {
        .mask = operand_mask(operand_type_mask::rm16, operand_type_mask::r16),
        .mandatory_prefix = mandatory_prefix::has_66,
        .operand_format = operand_format::mr,
        .opcode_format = opcode_format::plain_1,
        .opcode = {0x39U, 0x00U, 0x00U, 0x00U},
        .name = "66 39 /r",
      },
      {
        .mask = operand_mask(operand_type_mask::r16, operand_type_mask::rm16),
        .mandatory_prefix = mandatory_prefix::has_66,
        .operand_format = operand_format::rm,
        .opcode_format = opcode_format::plain_1,
        .opcode = {0x3BU, 0x00U, 0x00U, 0x00U},
        .name = "66 3B /r",
      },
      instruction_encoding{},
    },
  };
}
