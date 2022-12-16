#pragma once
// GENERATED HEADER

#include <lisp/assembly/amd64/instruction.hpp>

namespace j::lisp::assembly::amd64::inst {
  constexpr inline instruction<2> cvtsd2si = {
    .name      = "cvtsd2si",
    .encodings = {
      {
        .mask = operand_mask(operand_type_mask::r32, operand_type_mask::xmm_m64),
        .mandatory_prefix = mandatory_prefix::has_f2,
        .operand_format = operand_format::rm,
        .opcode_format = opcode_format::plain_2,
        .opcode = {0x0FU, 0x2DU, 0x00U, 0x00U},
        .name = "F2 0F 2D /r",
      },
      {
        .mask = operand_mask(operand_type_mask::r64, operand_type_mask::xmm_m64),
        .mandatory_prefix = mandatory_prefix::has_f2,
        .has_rex_w = true,
        .operand_format = operand_format::rm,
        .opcode_format = opcode_format::plain_2,
        .opcode = {0x0FU, 0x2DU, 0x00U, 0x00U},
        .name = "REX.W F2 0F 2D /r",
      },
      instruction_encoding{},
    },
  };
  constexpr inline instruction<1> cvtsd2ss = {
    .name      = "cvtsd2ss",
    .encodings = {
      {
        .mask = operand_mask(operand_type_mask::xmm, operand_type_mask::xmm_m64),
        .mandatory_prefix = mandatory_prefix::has_f2,
        .operand_format = operand_format::rm,
        .opcode_format = opcode_format::plain_2,
        .opcode = {0x0FU, 0x5AU, 0x00U, 0x00U},
        .name = "F2 0F 5A /r",
      },
      instruction_encoding{},
    },
  };
  constexpr inline instruction<2> cvtsi2sd = {
    .name      = "cvtsi2sd",
    .encodings = {
      {
        .mask = operand_mask(operand_type_mask::xmm, operand_type_mask::rm32),
        .mandatory_prefix = mandatory_prefix::has_f2,
        .operand_format = operand_format::rm,
        .opcode_format = opcode_format::plain_2,
        .opcode = {0x0FU, 0x2AU, 0x00U, 0x00U},
        .name = "F2 0F 2A /r",
      },
      {
        .mask = operand_mask(operand_type_mask::xmm, operand_type_mask::rm64),
        .mandatory_prefix = mandatory_prefix::has_f2,
        .has_rex_w = true,
        .operand_format = operand_format::rm,
        .opcode_format = opcode_format::plain_2,
        .opcode = {0x0FU, 0x2AU, 0x00U, 0x00U},
        .name = "REX.W F2 0F 2A /r",
      },
      instruction_encoding{},
    },
  };
  constexpr inline instruction<2> cvtsi2ss = {
    .name      = "cvtsi2ss",
    .encodings = {
      {
        .mask = operand_mask(operand_type_mask::xmm, operand_type_mask::rm32),
        .mandatory_prefix = mandatory_prefix::has_f3,
        .operand_format = operand_format::rm,
        .opcode_format = opcode_format::plain_2,
        .opcode = {0x0FU, 0x2AU, 0x00U, 0x00U},
        .name = "F3 0F 2A /r",
      },
      {
        .mask = operand_mask(operand_type_mask::xmm, operand_type_mask::rm64),
        .mandatory_prefix = mandatory_prefix::has_f3,
        .has_rex_w = true,
        .operand_format = operand_format::rm,
        .opcode_format = opcode_format::plain_2,
        .opcode = {0x0FU, 0x2AU, 0x00U, 0x00U},
        .name = "REX.W F3 0F 2A /r",
      },
      instruction_encoding{},
    },
  };
  constexpr inline instruction<1> cvtss2sd = {
    .name      = "cvtss2sd",
    .encodings = {
      {
        .mask = operand_mask(operand_type_mask::xmm, operand_type_mask::xmm_m32),
        .mandatory_prefix = mandatory_prefix::has_f3,
        .operand_format = operand_format::rm,
        .opcode_format = opcode_format::plain_2,
        .opcode = {0x0FU, 0x5AU, 0x00U, 0x00U},
        .name = "F3 0F 5A /r",
      },
      instruction_encoding{},
    },
  };
  constexpr inline instruction<2> cvtss2si = {
    .name      = "cvtss2si",
    .encodings = {
      {
        .mask = operand_mask(operand_type_mask::r32, operand_type_mask::xmm_m32),
        .mandatory_prefix = mandatory_prefix::has_f3,
        .operand_format = operand_format::rm,
        .opcode_format = opcode_format::plain_2,
        .opcode = {0x0FU, 0x2DU, 0x00U, 0x00U},
        .name = "F3 0F 2D /r",
      },
      {
        .mask = operand_mask(operand_type_mask::r64, operand_type_mask::xmm_m32),
        .mandatory_prefix = mandatory_prefix::has_f3,
        .has_rex_w = true,
        .operand_format = operand_format::rm,
        .opcode_format = opcode_format::plain_2,
        .opcode = {0x0FU, 0x2DU, 0x00U, 0x00U},
        .name = "REX.W F3 0F 2D /r",
      },
      instruction_encoding{},
    },
  };
}
