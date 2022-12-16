#pragma once
// GENERATED HEADER

#include <lisp/assembly/amd64/instruction.hpp>

namespace j::lisp::assembly::amd64::inst {
  constexpr inline instruction<14> xchg = {
    .name      = "xchg",
    .encodings = {
      {
        .mask = operand_mask(special_case_eax, operand_type_mask::r32),
        .operand_format = operand_format::implicit_ri,
        .opcode_format = opcode_format::plus_reg_1,
        .opcode = {0x90U, 0x00U, 0x00U, 0x00U},
        .name = "90+r",
      },
      {
        .mask = operand_mask(operand_type_mask::r32, special_case_eax),
        .operand_format = operand_format::ri_implicit,
        .opcode_format = opcode_format::plus_reg_1,
        .opcode = {0x90U, 0x00U, 0x00U, 0x00U},
        .name = "90+r",
      },
      {
        .mask = operand_mask(special_case_rax, operand_type_mask::r64),
        .has_rex_w = true,
        .operand_format = operand_format::implicit_ri,
        .opcode_format = opcode_format::plus_reg_1,
        .opcode = {0x90U, 0x00U, 0x00U, 0x00U},
        .name = "REX.W 90+r",
      },
      {
        .mask = operand_mask(operand_type_mask::r64, special_case_rax),
        .has_rex_w = true,
        .operand_format = operand_format::ri_implicit,
        .opcode_format = opcode_format::plus_reg_1,
        .opcode = {0x90U, 0x00U, 0x00U, 0x00U},
        .name = "REX.W 90+r",
      },
      {
        .mask = operand_mask(special_case_ax, operand_type_mask::r16),
        .mandatory_prefix = mandatory_prefix::has_66,
        .operand_format = operand_format::implicit_ri,
        .opcode_format = opcode_format::plus_reg_1,
        .opcode = {0x90U, 0x00U, 0x00U, 0x00U},
        .name = "66 90+r",
      },
      {
        .mask = operand_mask(operand_type_mask::r16, special_case_ax),
        .mandatory_prefix = mandatory_prefix::has_66,
        .operand_format = operand_format::ri_implicit,
        .opcode_format = opcode_format::plus_reg_1,
        .opcode = {0x90U, 0x00U, 0x00U, 0x00U},
        .name = "66 90+r",
      },
      {
        .mask = operand_mask(operand_type_mask::rm32, operand_type_mask::r32),
        .operand_format = operand_format::mr,
        .opcode_format = opcode_format::plain_1,
        .opcode = {0x87U, 0x00U, 0x00U, 0x00U},
        .name = "87 /r",
      },
      {
        .mask = operand_mask(operand_type_mask::r32, operand_type_mask::rm32),
        .operand_format = operand_format::rm,
        .opcode_format = opcode_format::plain_1,
        .opcode = {0x87U, 0x00U, 0x00U, 0x00U},
        .name = "87 /r",
      },
      {
        .mask = operand_mask(operand_type_mask::rm8, operand_type_mask::r8),
        .operand_format = operand_format::mr,
        .opcode_format = opcode_format::plain_1,
        .opcode = {0x86U, 0x00U, 0x00U, 0x00U},
        .name = "86 /r",
      },
      {
        .mask = operand_mask(operand_type_mask::r8, operand_type_mask::rm8),
        .operand_format = operand_format::rm,
        .opcode_format = opcode_format::plain_1,
        .opcode = {0x86U, 0x00U, 0x00U, 0x00U},
        .name = "86 /r",
      },
      {
        .mask = operand_mask(operand_type_mask::rm64, operand_type_mask::r64),
        .has_rex_w = true,
        .operand_format = operand_format::mr,
        .opcode_format = opcode_format::plain_1,
        .opcode = {0x87U, 0x00U, 0x00U, 0x00U},
        .name = "REX.W 87 /r",
      },
      {
        .mask = operand_mask(operand_type_mask::r64, operand_type_mask::rm64),
        .has_rex_w = true,
        .operand_format = operand_format::rm,
        .opcode_format = opcode_format::plain_1,
        .opcode = {0x87U, 0x00U, 0x00U, 0x00U},
        .name = "REX.W 87 /r",
      },
      {
        .mask = operand_mask(operand_type_mask::rm16, operand_type_mask::r16),
        .mandatory_prefix = mandatory_prefix::has_66,
        .operand_format = operand_format::mr,
        .opcode_format = opcode_format::plain_1,
        .opcode = {0x87U, 0x00U, 0x00U, 0x00U},
        .name = "66 87 /r",
      },
      {
        .mask = operand_mask(operand_type_mask::r16, operand_type_mask::rm16),
        .mandatory_prefix = mandatory_prefix::has_66,
        .operand_format = operand_format::rm,
        .opcode_format = opcode_format::plain_1,
        .opcode = {0x87U, 0x00U, 0x00U, 0x00U},
        .name = "66 87 /r",
      },
      instruction_encoding{},
    },
  };
}
