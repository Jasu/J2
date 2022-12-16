#pragma once

#include "lisp/assembly/amd64/instruction.hpp"

namespace j::lisp::assembly::amd64::inst {
  constexpr inline instruction<1> ret = {
    .name      = "ret",
    .encodings = {
      {
        .mask = operand_mask(),
        .opcode_format = opcode_format::plain_1,
        .opcode = {0xC3U, 0x00U, 0x00U, 0x00U},
        .name = "C3",
      },
      instruction_encoding{},
    },
  };

}
