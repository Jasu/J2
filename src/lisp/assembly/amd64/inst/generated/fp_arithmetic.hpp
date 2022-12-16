#pragma once
// GENERATED HEADER

#include <lisp/assembly/amd64/instruction.hpp>

namespace j::lisp::assembly::amd64::inst {
  constexpr inline instruction<1> addsd = {
    .name      = "addsd",
    .encodings = {
      {
        .mask = operand_mask(operand_type_mask::xmm, operand_type_mask::xmm_m64),
        .mandatory_prefix = mandatory_prefix::has_f2,
        .operand_format = operand_format::rm,
        .opcode_format = opcode_format::plain_2,
        .opcode = {0x0FU, 0x58U, 0x00U, 0x00U},
        .name = "F2 0F 58 /r",
      },
      instruction_encoding{},
    },
  };
  constexpr inline instruction<1> addss = {
    .name      = "addss",
    .encodings = {
      {
        .mask = operand_mask(operand_type_mask::xmm, operand_type_mask::xmm_m32),
        .mandatory_prefix = mandatory_prefix::has_f3,
        .operand_format = operand_format::rm,
        .opcode_format = opcode_format::plain_2,
        .opcode = {0x0FU, 0x58U, 0x00U, 0x00U},
        .name = "F3 0F 58 /r",
      },
      instruction_encoding{},
    },
  };
  constexpr inline instruction<1> subsd = {
    .name      = "subsd",
    .encodings = {
      {
        .mask = operand_mask(operand_type_mask::xmm, operand_type_mask::xmm_m64),
        .mandatory_prefix = mandatory_prefix::has_f2,
        .operand_format = operand_format::rm,
        .opcode_format = opcode_format::plain_2,
        .opcode = {0x0FU, 0x5CU, 0x00U, 0x00U},
        .name = "F2 0F 5C /r",
      },
      instruction_encoding{},
    },
  };
  constexpr inline instruction<1> subss = {
    .name      = "subss",
    .encodings = {
      {
        .mask = operand_mask(operand_type_mask::xmm, operand_type_mask::xmm_m32),
        .mandatory_prefix = mandatory_prefix::has_f3,
        .operand_format = operand_format::rm,
        .opcode_format = opcode_format::plain_2,
        .opcode = {0x0FU, 0x5CU, 0x00U, 0x00U},
        .name = "F3 0F 5C /r",
      },
      instruction_encoding{},
    },
  };
}
