#pragma once
// GENERATED HEADER

#include <lisp/assembly/amd64/instruction.hpp>

namespace j::lisp::assembly::amd64::inst {
  constexpr inline instruction<1> int3 = {
    .name      = "int3",
    .encodings = {
      {
        .mask = operand_mask(),
        .operand_format = operand_format::none,
        .opcode_format = opcode_format::plain_1,
        .opcode = {0xCCU, 0x00U, 0x00U, 0x00U},
        .name = "CC",
      },
      instruction_encoding{},
    },
  };
}
