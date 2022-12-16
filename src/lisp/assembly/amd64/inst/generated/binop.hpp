#pragma once
// GENERATED HEADER

#include <lisp/assembly/amd64/instruction.hpp>

namespace j::lisp::assembly::amd64::inst {
  constexpr inline instruction<19> band = {
    .name      = "and",
    .encodings = {
      {
        .mask = operand_mask(special_case_eax, operand_type_mask::imm32),
        .operand_format = operand_format::implicit,
        .opcode_format = opcode_format::plain_1,
        .imm_format = imm_format::imm32,
        .opcode = {0x25U, 0x00U, 0x00U, 0x00U},
        .name = "25 id",
      },
      {
        .mask = operand_mask(operand_type_mask::rm32, operand_type_mask::imm32),
        .operand_format = operand_format::m,
        .opcode_format = opcode_format::plain_1,
        .modrm_forced_r = 4,
        .imm_format = imm_format::imm32,
        .opcode = {0x81U, 0x00U, 0x00U, 0x00U},
        .name = "81 /4 id",
      },
      {
        .mask = operand_mask(operand_type_mask::rm32, operand_type_mask::imm8),
        .operand_format = operand_format::m,
        .opcode_format = opcode_format::plain_1,
        .modrm_forced_r = 4,
        .imm_format = imm_format::imm8,
        .opcode = {0x83U, 0x00U, 0x00U, 0x00U},
        .name = "83 /4 ib",
      },
      {
        .mask = operand_mask(operand_type_mask::rm32, operand_type_mask::r32),
        .operand_format = operand_format::mr,
        .opcode_format = opcode_format::plain_1,
        .opcode = {0x21U, 0x00U, 0x00U, 0x00U},
        .name = "21 /r",
      },
      {
        .mask = operand_mask(operand_type_mask::r32, operand_type_mask::rm32),
        .operand_format = operand_format::rm,
        .opcode_format = opcode_format::plain_1,
        .opcode = {0x23U, 0x00U, 0x00U, 0x00U},
        .name = "23 /r",
      },
      {
        .mask = operand_mask(special_case_al, operand_type_mask::imm8),
        .operand_format = operand_format::implicit,
        .opcode_format = opcode_format::plain_1,
        .imm_format = imm_format::imm8,
        .opcode = {0x24U, 0x00U, 0x00U, 0x00U},
        .name = "24 ib",
      },
      {
        .mask = operand_mask(operand_type_mask::rm8, operand_type_mask::imm8),
        .operand_format = operand_format::m,
        .opcode_format = opcode_format::plain_1,
        .modrm_forced_r = 4,
        .imm_format = imm_format::imm8,
        .opcode = {0x80U, 0x00U, 0x00U, 0x00U},
        .name = "80 /4 ib",
      },
      {
        .mask = operand_mask(operand_type_mask::rm8, operand_type_mask::r8),
        .operand_format = operand_format::mr,
        .opcode_format = opcode_format::plain_1,
        .opcode = {0x20U, 0x00U, 0x00U, 0x00U},
        .name = "20 /r",
      },
      {
        .mask = operand_mask(operand_type_mask::r8, operand_type_mask::rm8),
        .operand_format = operand_format::rm,
        .opcode_format = opcode_format::plain_1,
        .opcode = {0x22U, 0x00U, 0x00U, 0x00U},
        .name = "22 /r",
      },
      {
        .mask = operand_mask(special_case_rax, operand_type_mask::imm32),
        .has_rex_w = true,
        .operand_format = operand_format::implicit,
        .opcode_format = opcode_format::plain_1,
        .imm_format = imm_format::imm32,
        .opcode = {0x25U, 0x00U, 0x00U, 0x00U},
        .name = "REX.W 25 id",
      },
      {
        .mask = operand_mask(operand_type_mask::rm64, operand_type_mask::imm32),
        .has_rex_w = true,
        .operand_format = operand_format::m,
        .opcode_format = opcode_format::plain_1,
        .modrm_forced_r = 4,
        .imm_format = imm_format::imm32,
        .opcode = {0x81U, 0x00U, 0x00U, 0x00U},
        .name = "REX.W 81 /4 id",
      },
      {
        .mask = operand_mask(operand_type_mask::rm64, operand_type_mask::imm8),
        .has_rex_w = true,
        .operand_format = operand_format::m,
        .opcode_format = opcode_format::plain_1,
        .modrm_forced_r = 4,
        .imm_format = imm_format::imm8,
        .opcode = {0x83U, 0x00U, 0x00U, 0x00U},
        .name = "REX.W 83 /4 ib",
      },
      {
        .mask = operand_mask(operand_type_mask::rm64, operand_type_mask::r64),
        .has_rex_w = true,
        .operand_format = operand_format::mr,
        .opcode_format = opcode_format::plain_1,
        .opcode = {0x21U, 0x00U, 0x00U, 0x00U},
        .name = "REX.W 21 /r",
      },
      {
        .mask = operand_mask(operand_type_mask::r64, operand_type_mask::rm64),
        .has_rex_w = true,
        .operand_format = operand_format::rm,
        .opcode_format = opcode_format::plain_1,
        .opcode = {0x23U, 0x00U, 0x00U, 0x00U},
        .name = "REX.W 23 /r",
      },
      {
        .mask = operand_mask(special_case_ax, operand_type_mask::imm16),
        .mandatory_prefix = mandatory_prefix::has_66,
        .operand_format = operand_format::implicit,
        .opcode_format = opcode_format::plain_1,
        .imm_format = imm_format::imm16,
        .opcode = {0x25U, 0x00U, 0x00U, 0x00U},
        .name = "66 25 iw",
      },
      {
        .mask = operand_mask(operand_type_mask::rm16, operand_type_mask::imm16),
        .mandatory_prefix = mandatory_prefix::has_66,
        .operand_format = operand_format::m,
        .opcode_format = opcode_format::plain_1,
        .modrm_forced_r = 4,
        .imm_format = imm_format::imm16,
        .opcode = {0x81U, 0x00U, 0x00U, 0x00U},
        .name = "66 81 /4 iw",
      },
      {
        .mask = operand_mask(operand_type_mask::rm16, operand_type_mask::imm8),
        .mandatory_prefix = mandatory_prefix::has_66,
        .operand_format = operand_format::m,
        .opcode_format = opcode_format::plain_1,
        .modrm_forced_r = 4,
        .imm_format = imm_format::imm8,
        .opcode = {0x83U, 0x00U, 0x00U, 0x00U},
        .name = "66 83 /4 ib",
      },
      {
        .mask = operand_mask(operand_type_mask::rm16, operand_type_mask::r16),
        .mandatory_prefix = mandatory_prefix::has_66,
        .operand_format = operand_format::mr,
        .opcode_format = opcode_format::plain_1,
        .opcode = {0x21U, 0x00U, 0x00U, 0x00U},
        .name = "66 21 /r",
      },
      {
        .mask = operand_mask(operand_type_mask::r16, operand_type_mask::rm16),
        .mandatory_prefix = mandatory_prefix::has_66,
        .operand_format = operand_format::rm,
        .opcode_format = opcode_format::plain_1,
        .opcode = {0x23U, 0x00U, 0x00U, 0x00U},
        .name = "66 23 /r",
      },
      instruction_encoding{},
    },
  };
  constexpr inline instruction<4> bnot = {
    .name      = "not",
    .encodings = {
      {
        .mask = operand_mask(operand_type_mask::rm32),
        .operand_format = operand_format::m,
        .opcode_format = opcode_format::plain_1,
        .modrm_forced_r = 2,
        .opcode = {0xF7U, 0x00U, 0x00U, 0x00U},
        .name = "F7 /2",
      },
      {
        .mask = operand_mask(operand_type_mask::rm8),
        .operand_format = operand_format::m,
        .opcode_format = opcode_format::plain_1,
        .modrm_forced_r = 2,
        .opcode = {0xF6U, 0x00U, 0x00U, 0x00U},
        .name = "F6 /2",
      },
      {
        .mask = operand_mask(operand_type_mask::rm64),
        .has_rex_w = true,
        .operand_format = operand_format::m,
        .opcode_format = opcode_format::plain_1,
        .modrm_forced_r = 2,
        .opcode = {0xF7U, 0x00U, 0x00U, 0x00U},
        .name = "REX.W F7 /2",
      },
      {
        .mask = operand_mask(operand_type_mask::rm16),
        .mandatory_prefix = mandatory_prefix::has_66,
        .operand_format = operand_format::m,
        .opcode_format = opcode_format::plain_1,
        .modrm_forced_r = 2,
        .opcode = {0xF7U, 0x00U, 0x00U, 0x00U},
        .name = "66 F7 /2",
      },
      instruction_encoding{},
    },
  };
  constexpr inline instruction<19> bor = {
    .name      = "or",
    .encodings = {
      {
        .mask = operand_mask(special_case_eax, operand_type_mask::imm32),
        .operand_format = operand_format::implicit,
        .opcode_format = opcode_format::plain_1,
        .imm_format = imm_format::imm32,
        .opcode = {0x0DU, 0x00U, 0x00U, 0x00U},
        .name = "0D id",
      },
      {
        .mask = operand_mask(operand_type_mask::rm32, operand_type_mask::imm32),
        .operand_format = operand_format::m,
        .opcode_format = opcode_format::plain_1,
        .modrm_forced_r = 1,
        .imm_format = imm_format::imm32,
        .opcode = {0x81U, 0x00U, 0x00U, 0x00U},
        .name = "81 /1 id",
      },
      {
        .mask = operand_mask(operand_type_mask::rm32, operand_type_mask::imm8),
        .operand_format = operand_format::m,
        .opcode_format = opcode_format::plain_1,
        .modrm_forced_r = 1,
        .imm_format = imm_format::imm8,
        .opcode = {0x83U, 0x00U, 0x00U, 0x00U},
        .name = "83 /1 ib",
      },
      {
        .mask = operand_mask(operand_type_mask::rm32, operand_type_mask::r32),
        .operand_format = operand_format::mr,
        .opcode_format = opcode_format::plain_1,
        .opcode = {0x09U, 0x00U, 0x00U, 0x00U},
        .name = "09 /r",
      },
      {
        .mask = operand_mask(operand_type_mask::r32, operand_type_mask::rm32),
        .operand_format = operand_format::rm,
        .opcode_format = opcode_format::plain_1,
        .opcode = {0x0BU, 0x00U, 0x00U, 0x00U},
        .name = "0B /r",
      },
      {
        .mask = operand_mask(special_case_al, operand_type_mask::imm8),
        .operand_format = operand_format::implicit,
        .opcode_format = opcode_format::plain_1,
        .imm_format = imm_format::imm8,
        .opcode = {0x0CU, 0x00U, 0x00U, 0x00U},
        .name = "0C ib",
      },
      {
        .mask = operand_mask(operand_type_mask::rm8, operand_type_mask::imm8),
        .operand_format = operand_format::m,
        .opcode_format = opcode_format::plain_1,
        .modrm_forced_r = 1,
        .imm_format = imm_format::imm8,
        .opcode = {0x80U, 0x00U, 0x00U, 0x00U},
        .name = "80 /1 ib",
      },
      {
        .mask = operand_mask(operand_type_mask::rm8, operand_type_mask::r8),
        .operand_format = operand_format::mr,
        .opcode_format = opcode_format::plain_1,
        .opcode = {0x08U, 0x00U, 0x00U, 0x00U},
        .name = "08 /r",
      },
      {
        .mask = operand_mask(operand_type_mask::r8, operand_type_mask::rm8),
        .operand_format = operand_format::rm,
        .opcode_format = opcode_format::plain_1,
        .opcode = {0x0AU, 0x00U, 0x00U, 0x00U},
        .name = "0A /r",
      },
      {
        .mask = operand_mask(special_case_rax, operand_type_mask::imm32),
        .has_rex_w = true,
        .operand_format = operand_format::implicit,
        .opcode_format = opcode_format::plain_1,
        .imm_format = imm_format::imm32,
        .opcode = {0x0DU, 0x00U, 0x00U, 0x00U},
        .name = "REX.W 0D id",
      },
      {
        .mask = operand_mask(operand_type_mask::rm64, operand_type_mask::imm32),
        .has_rex_w = true,
        .operand_format = operand_format::m,
        .opcode_format = opcode_format::plain_1,
        .modrm_forced_r = 1,
        .imm_format = imm_format::imm32,
        .opcode = {0x81U, 0x00U, 0x00U, 0x00U},
        .name = "REX.W 81 /1 id",
      },
      {
        .mask = operand_mask(operand_type_mask::rm64, operand_type_mask::imm8),
        .has_rex_w = true,
        .operand_format = operand_format::m,
        .opcode_format = opcode_format::plain_1,
        .modrm_forced_r = 1,
        .imm_format = imm_format::imm8,
        .opcode = {0x83U, 0x00U, 0x00U, 0x00U},
        .name = "REX.W 83 /1 ib",
      },
      {
        .mask = operand_mask(operand_type_mask::rm64, operand_type_mask::r64),
        .has_rex_w = true,
        .operand_format = operand_format::mr,
        .opcode_format = opcode_format::plain_1,
        .opcode = {0x09U, 0x00U, 0x00U, 0x00U},
        .name = "REX.W 09 /r",
      },
      {
        .mask = operand_mask(operand_type_mask::r64, operand_type_mask::rm64),
        .has_rex_w = true,
        .operand_format = operand_format::rm,
        .opcode_format = opcode_format::plain_1,
        .opcode = {0x0BU, 0x00U, 0x00U, 0x00U},
        .name = "REX.W 0B /r",
      },
      {
        .mask = operand_mask(special_case_ax, operand_type_mask::imm16),
        .mandatory_prefix = mandatory_prefix::has_66,
        .operand_format = operand_format::implicit,
        .opcode_format = opcode_format::plain_1,
        .imm_format = imm_format::imm16,
        .opcode = {0x0DU, 0x00U, 0x00U, 0x00U},
        .name = "66 0D iw",
      },
      {
        .mask = operand_mask(operand_type_mask::rm16, operand_type_mask::imm16),
        .mandatory_prefix = mandatory_prefix::has_66,
        .operand_format = operand_format::m,
        .opcode_format = opcode_format::plain_1,
        .modrm_forced_r = 1,
        .imm_format = imm_format::imm16,
        .opcode = {0x81U, 0x00U, 0x00U, 0x00U},
        .name = "66 81 /1 iw",
      },
      {
        .mask = operand_mask(operand_type_mask::rm16, operand_type_mask::imm8),
        .mandatory_prefix = mandatory_prefix::has_66,
        .operand_format = operand_format::m,
        .opcode_format = opcode_format::plain_1,
        .modrm_forced_r = 1,
        .imm_format = imm_format::imm8,
        .opcode = {0x83U, 0x00U, 0x00U, 0x00U},
        .name = "66 83 /1 ib",
      },
      {
        .mask = operand_mask(operand_type_mask::rm16, operand_type_mask::r16),
        .mandatory_prefix = mandatory_prefix::has_66,
        .operand_format = operand_format::mr,
        .opcode_format = opcode_format::plain_1,
        .opcode = {0x09U, 0x00U, 0x00U, 0x00U},
        .name = "66 09 /r",
      },
      {
        .mask = operand_mask(operand_type_mask::r16, operand_type_mask::rm16),
        .mandatory_prefix = mandatory_prefix::has_66,
        .operand_format = operand_format::rm,
        .opcode_format = opcode_format::plain_1,
        .opcode = {0x0BU, 0x00U, 0x00U, 0x00U},
        .name = "66 0B /r",
      },
      instruction_encoding{},
    },
  };
  constexpr inline instruction<19> bxor = {
    .name      = "xor",
    .encodings = {
      {
        .mask = operand_mask(special_case_eax, operand_type_mask::imm32),
        .operand_format = operand_format::implicit,
        .opcode_format = opcode_format::plain_1,
        .imm_format = imm_format::imm32,
        .opcode = {0x35U, 0x00U, 0x00U, 0x00U},
        .name = "35 id",
      },
      {
        .mask = operand_mask(operand_type_mask::rm32, operand_type_mask::imm32),
        .operand_format = operand_format::m,
        .opcode_format = opcode_format::plain_1,
        .modrm_forced_r = 6,
        .imm_format = imm_format::imm32,
        .opcode = {0x81U, 0x00U, 0x00U, 0x00U},
        .name = "81 /6 id",
      },
      {
        .mask = operand_mask(operand_type_mask::rm32, operand_type_mask::imm8),
        .operand_format = operand_format::m,
        .opcode_format = opcode_format::plain_1,
        .modrm_forced_r = 6,
        .imm_format = imm_format::imm8,
        .opcode = {0x83U, 0x00U, 0x00U, 0x00U},
        .name = "83 /6 ib",
      },
      {
        .mask = operand_mask(operand_type_mask::rm32, operand_type_mask::r32),
        .operand_format = operand_format::mr,
        .opcode_format = opcode_format::plain_1,
        .opcode = {0x31U, 0x00U, 0x00U, 0x00U},
        .name = "31 /r",
      },
      {
        .mask = operand_mask(operand_type_mask::r32, operand_type_mask::rm32),
        .operand_format = operand_format::rm,
        .opcode_format = opcode_format::plain_1,
        .opcode = {0x33U, 0x00U, 0x00U, 0x00U},
        .name = "33 /r",
      },
      {
        .mask = operand_mask(special_case_al, operand_type_mask::imm8),
        .operand_format = operand_format::implicit,
        .opcode_format = opcode_format::plain_1,
        .imm_format = imm_format::imm8,
        .opcode = {0x34U, 0x00U, 0x00U, 0x00U},
        .name = "34 ib",
      },
      {
        .mask = operand_mask(operand_type_mask::rm8, operand_type_mask::imm8),
        .operand_format = operand_format::m,
        .opcode_format = opcode_format::plain_1,
        .modrm_forced_r = 6,
        .imm_format = imm_format::imm8,
        .opcode = {0x80U, 0x00U, 0x00U, 0x00U},
        .name = "80 /6 ib",
      },
      {
        .mask = operand_mask(operand_type_mask::rm8, operand_type_mask::r8),
        .operand_format = operand_format::mr,
        .opcode_format = opcode_format::plain_1,
        .opcode = {0x30U, 0x00U, 0x00U, 0x00U},
        .name = "30 /r",
      },
      {
        .mask = operand_mask(operand_type_mask::r8, operand_type_mask::rm8),
        .operand_format = operand_format::rm,
        .opcode_format = opcode_format::plain_1,
        .opcode = {0x32U, 0x00U, 0x00U, 0x00U},
        .name = "32 /r",
      },
      {
        .mask = operand_mask(special_case_rax, operand_type_mask::imm32),
        .has_rex_w = true,
        .operand_format = operand_format::implicit,
        .opcode_format = opcode_format::plain_1,
        .imm_format = imm_format::imm32,
        .opcode = {0x35U, 0x00U, 0x00U, 0x00U},
        .name = "REX.W 35 id",
      },
      {
        .mask = operand_mask(operand_type_mask::rm64, operand_type_mask::imm32),
        .has_rex_w = true,
        .operand_format = operand_format::m,
        .opcode_format = opcode_format::plain_1,
        .modrm_forced_r = 6,
        .imm_format = imm_format::imm32,
        .opcode = {0x81U, 0x00U, 0x00U, 0x00U},
        .name = "REX.W 81 /6 id",
      },
      {
        .mask = operand_mask(operand_type_mask::rm64, operand_type_mask::imm8),
        .has_rex_w = true,
        .operand_format = operand_format::m,
        .opcode_format = opcode_format::plain_1,
        .modrm_forced_r = 6,
        .imm_format = imm_format::imm8,
        .opcode = {0x83U, 0x00U, 0x00U, 0x00U},
        .name = "REX.W 83 /6 ib",
      },
      {
        .mask = operand_mask(operand_type_mask::rm64, operand_type_mask::r64),
        .has_rex_w = true,
        .operand_format = operand_format::mr,
        .opcode_format = opcode_format::plain_1,
        .opcode = {0x31U, 0x00U, 0x00U, 0x00U},
        .name = "REX.W 31 /r",
      },
      {
        .mask = operand_mask(operand_type_mask::r64, operand_type_mask::rm64),
        .has_rex_w = true,
        .operand_format = operand_format::rm,
        .opcode_format = opcode_format::plain_1,
        .opcode = {0x33U, 0x00U, 0x00U, 0x00U},
        .name = "REX.W 33 /r",
      },
      {
        .mask = operand_mask(special_case_ax, operand_type_mask::imm16),
        .mandatory_prefix = mandatory_prefix::has_66,
        .operand_format = operand_format::implicit,
        .opcode_format = opcode_format::plain_1,
        .imm_format = imm_format::imm16,
        .opcode = {0x35U, 0x00U, 0x00U, 0x00U},
        .name = "66 35 iw",
      },
      {
        .mask = operand_mask(operand_type_mask::rm16, operand_type_mask::imm16),
        .mandatory_prefix = mandatory_prefix::has_66,
        .operand_format = operand_format::m,
        .opcode_format = opcode_format::plain_1,
        .modrm_forced_r = 6,
        .imm_format = imm_format::imm16,
        .opcode = {0x81U, 0x00U, 0x00U, 0x00U},
        .name = "66 81 /6 iw",
      },
      {
        .mask = operand_mask(operand_type_mask::rm16, operand_type_mask::imm8),
        .mandatory_prefix = mandatory_prefix::has_66,
        .operand_format = operand_format::m,
        .opcode_format = opcode_format::plain_1,
        .modrm_forced_r = 6,
        .imm_format = imm_format::imm8,
        .opcode = {0x83U, 0x00U, 0x00U, 0x00U},
        .name = "66 83 /6 ib",
      },
      {
        .mask = operand_mask(operand_type_mask::rm16, operand_type_mask::r16),
        .mandatory_prefix = mandatory_prefix::has_66,
        .operand_format = operand_format::mr,
        .opcode_format = opcode_format::plain_1,
        .opcode = {0x31U, 0x00U, 0x00U, 0x00U},
        .name = "66 31 /r",
      },
      {
        .mask = operand_mask(operand_type_mask::r16, operand_type_mask::rm16),
        .mandatory_prefix = mandatory_prefix::has_66,
        .operand_format = operand_format::rm,
        .opcode_format = opcode_format::plain_1,
        .opcode = {0x33U, 0x00U, 0x00U, 0x00U},
        .name = "66 33 /r",
      },
      instruction_encoding{},
    },
  };
}
