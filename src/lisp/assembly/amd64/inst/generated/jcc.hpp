#pragma once
// GENERATED HEADER

#include <lisp/assembly/amd64/instruction.hpp>

namespace j::lisp::assembly::amd64::inst {
  constexpr inline instruction<2> ja = {
    .name      = "ja",
    .encodings = {
      {
        .mask = operand_mask(operand_type_mask::imm8),
        .operand_format = operand_format::none,
        .opcode_format = opcode_format::plain_1,
        .imm_format = imm_format::rel8,
        .opcode = {0x77U, 0x00U, 0x00U, 0x00U},
        .name = "77 cb",
      },
      {
        .mask = operand_mask(operand_type_mask::imm32),
        .operand_format = operand_format::none,
        .opcode_format = opcode_format::plain_2,
        .imm_format = imm_format::rel32,
        .opcode = {0x0FU, 0x87U, 0x00U, 0x00U},
        .name = "0F 87 cd",
      },
      instruction_encoding{},
    },
  };
  constexpr inline instruction<2> jae = {
    .name      = "jae",
    .encodings = {
      {
        .mask = operand_mask(operand_type_mask::imm8),
        .operand_format = operand_format::none,
        .opcode_format = opcode_format::plain_1,
        .imm_format = imm_format::rel8,
        .opcode = {0x73U, 0x00U, 0x00U, 0x00U},
        .name = "73 cb",
      },
      {
        .mask = operand_mask(operand_type_mask::imm32),
        .operand_format = operand_format::none,
        .opcode_format = opcode_format::plain_2,
        .imm_format = imm_format::rel32,
        .opcode = {0x0FU, 0x83U, 0x00U, 0x00U},
        .name = "0F 83 cd",
      },
      instruction_encoding{},
    },
  };
  constexpr inline instruction<2> jb = {
    .name      = "jb",
    .encodings = {
      {
        .mask = operand_mask(operand_type_mask::imm8),
        .operand_format = operand_format::none,
        .opcode_format = opcode_format::plain_1,
        .imm_format = imm_format::rel8,
        .opcode = {0x72U, 0x00U, 0x00U, 0x00U},
        .name = "72 cb",
      },
      {
        .mask = operand_mask(operand_type_mask::imm32),
        .operand_format = operand_format::none,
        .opcode_format = opcode_format::plain_2,
        .imm_format = imm_format::rel32,
        .opcode = {0x0FU, 0x82U, 0x00U, 0x00U},
        .name = "0F 82 cd",
      },
      instruction_encoding{},
    },
  };
  constexpr inline instruction<2> jbe = {
    .name      = "jbe",
    .encodings = {
      {
        .mask = operand_mask(operand_type_mask::imm8),
        .operand_format = operand_format::none,
        .opcode_format = opcode_format::plain_1,
        .imm_format = imm_format::rel8,
        .opcode = {0x76U, 0x00U, 0x00U, 0x00U},
        .name = "76 cb",
      },
      {
        .mask = operand_mask(operand_type_mask::imm32),
        .operand_format = operand_format::none,
        .opcode_format = opcode_format::plain_2,
        .imm_format = imm_format::rel32,
        .opcode = {0x0FU, 0x86U, 0x00U, 0x00U},
        .name = "0F 86 cd",
      },
      instruction_encoding{},
    },
  };
  constexpr inline instruction<2> je = {
    .name      = "je",
    .encodings = {
      {
        .mask = operand_mask(operand_type_mask::imm8),
        .operand_format = operand_format::none,
        .opcode_format = opcode_format::plain_1,
        .imm_format = imm_format::rel8,
        .opcode = {0x74U, 0x00U, 0x00U, 0x00U},
        .name = "74 cb",
      },
      {
        .mask = operand_mask(operand_type_mask::imm32),
        .operand_format = operand_format::none,
        .opcode_format = opcode_format::plain_2,
        .imm_format = imm_format::rel32,
        .opcode = {0x0FU, 0x84U, 0x00U, 0x00U},
        .name = "0F 84 cd",
      },
      instruction_encoding{},
    },
  };
  constexpr inline instruction<2> jg = {
    .name      = "jg",
    .encodings = {
      {
        .mask = operand_mask(operand_type_mask::imm8),
        .operand_format = operand_format::none,
        .opcode_format = opcode_format::plain_1,
        .imm_format = imm_format::rel8,
        .opcode = {0x7FU, 0x00U, 0x00U, 0x00U},
        .name = "7F cb",
      },
      {
        .mask = operand_mask(operand_type_mask::imm32),
        .operand_format = operand_format::none,
        .opcode_format = opcode_format::plain_2,
        .imm_format = imm_format::rel32,
        .opcode = {0x0FU, 0x8FU, 0x00U, 0x00U},
        .name = "0F 8F cd",
      },
      instruction_encoding{},
    },
  };
  constexpr inline instruction<2> jge = {
    .name      = "jge",
    .encodings = {
      {
        .mask = operand_mask(operand_type_mask::imm8),
        .operand_format = operand_format::none,
        .opcode_format = opcode_format::plain_1,
        .imm_format = imm_format::rel8,
        .opcode = {0x7DU, 0x00U, 0x00U, 0x00U},
        .name = "7D cb",
      },
      {
        .mask = operand_mask(operand_type_mask::imm32),
        .operand_format = operand_format::none,
        .opcode_format = opcode_format::plain_2,
        .imm_format = imm_format::rel32,
        .opcode = {0x0FU, 0x8DU, 0x00U, 0x00U},
        .name = "0F 8D cd",
      },
      instruction_encoding{},
    },
  };
  constexpr inline instruction<2> jl = {
    .name      = "jl",
    .encodings = {
      {
        .mask = operand_mask(operand_type_mask::imm8),
        .operand_format = operand_format::none,
        .opcode_format = opcode_format::plain_1,
        .imm_format = imm_format::rel8,
        .opcode = {0x7CU, 0x00U, 0x00U, 0x00U},
        .name = "7C cb",
      },
      {
        .mask = operand_mask(operand_type_mask::imm32),
        .operand_format = operand_format::none,
        .opcode_format = opcode_format::plain_2,
        .imm_format = imm_format::rel32,
        .opcode = {0x0FU, 0x8CU, 0x00U, 0x00U},
        .name = "0F 8C cd",
      },
      instruction_encoding{},
    },
  };
  constexpr inline instruction<2> jle = {
    .name      = "jle",
    .encodings = {
      {
        .mask = operand_mask(operand_type_mask::imm8),
        .operand_format = operand_format::none,
        .opcode_format = opcode_format::plain_1,
        .imm_format = imm_format::rel8,
        .opcode = {0x7EU, 0x00U, 0x00U, 0x00U},
        .name = "7E cb",
      },
      {
        .mask = operand_mask(operand_type_mask::imm32),
        .operand_format = operand_format::none,
        .opcode_format = opcode_format::plain_2,
        .imm_format = imm_format::rel32,
        .opcode = {0x0FU, 0x8EU, 0x00U, 0x00U},
        .name = "0F 8E cd",
      },
      instruction_encoding{},
    },
  };
  constexpr inline instruction<2> jne = {
    .name      = "jne",
    .encodings = {
      {
        .mask = operand_mask(operand_type_mask::imm8),
        .operand_format = operand_format::none,
        .opcode_format = opcode_format::plain_1,
        .imm_format = imm_format::rel8,
        .opcode = {0x75U, 0x00U, 0x00U, 0x00U},
        .name = "75 cb",
      },
      {
        .mask = operand_mask(operand_type_mask::imm32),
        .operand_format = operand_format::none,
        .opcode_format = opcode_format::plain_2,
        .imm_format = imm_format::rel32,
        .opcode = {0x0FU, 0x85U, 0x00U, 0x00U},
        .name = "0F 85 cd",
      },
      instruction_encoding{},
    },
  };
  constexpr inline instruction<2> jno = {
    .name      = "jno",
    .encodings = {
      {
        .mask = operand_mask(operand_type_mask::imm8),
        .operand_format = operand_format::none,
        .opcode_format = opcode_format::plain_1,
        .imm_format = imm_format::rel8,
        .opcode = {0x71U, 0x00U, 0x00U, 0x00U},
        .name = "71 cb",
      },
      {
        .mask = operand_mask(operand_type_mask::imm32),
        .operand_format = operand_format::none,
        .opcode_format = opcode_format::plain_2,
        .imm_format = imm_format::rel32,
        .opcode = {0x0FU, 0x81U, 0x00U, 0x00U},
        .name = "0F 81 cd",
      },
      instruction_encoding{},
    },
  };
  constexpr inline instruction<2> jnp = {
    .name      = "jnp",
    .encodings = {
      {
        .mask = operand_mask(operand_type_mask::imm8),
        .operand_format = operand_format::none,
        .opcode_format = opcode_format::plain_1,
        .imm_format = imm_format::rel8,
        .opcode = {0x7BU, 0x00U, 0x00U, 0x00U},
        .name = "7B cb",
      },
      {
        .mask = operand_mask(operand_type_mask::imm32),
        .operand_format = operand_format::none,
        .opcode_format = opcode_format::plain_2,
        .imm_format = imm_format::rel32,
        .opcode = {0x0FU, 0x8BU, 0x00U, 0x00U},
        .name = "0F 8B cd",
      },
      instruction_encoding{},
    },
  };
  constexpr inline instruction<2> jns = {
    .name      = "jns",
    .encodings = {
      {
        .mask = operand_mask(operand_type_mask::imm8),
        .operand_format = operand_format::none,
        .opcode_format = opcode_format::plain_1,
        .imm_format = imm_format::rel8,
        .opcode = {0x79U, 0x00U, 0x00U, 0x00U},
        .name = "79 cb",
      },
      {
        .mask = operand_mask(operand_type_mask::imm32),
        .operand_format = operand_format::none,
        .opcode_format = opcode_format::plain_2,
        .imm_format = imm_format::rel32,
        .opcode = {0x0FU, 0x89U, 0x00U, 0x00U},
        .name = "0F 89 cd",
      },
      instruction_encoding{},
    },
  };
  constexpr inline instruction<2> jo = {
    .name      = "jo",
    .encodings = {
      {
        .mask = operand_mask(operand_type_mask::imm8),
        .operand_format = operand_format::none,
        .opcode_format = opcode_format::plain_1,
        .imm_format = imm_format::rel8,
        .opcode = {0x70U, 0x00U, 0x00U, 0x00U},
        .name = "70 cb",
      },
      {
        .mask = operand_mask(operand_type_mask::imm32),
        .operand_format = operand_format::none,
        .opcode_format = opcode_format::plain_2,
        .imm_format = imm_format::rel32,
        .opcode = {0x0FU, 0x80U, 0x00U, 0x00U},
        .name = "0F 80 cd",
      },
      instruction_encoding{},
    },
  };
  constexpr inline instruction<2> jp = {
    .name      = "jp",
    .encodings = {
      {
        .mask = operand_mask(operand_type_mask::imm8),
        .operand_format = operand_format::none,
        .opcode_format = opcode_format::plain_1,
        .imm_format = imm_format::rel8,
        .opcode = {0x7AU, 0x00U, 0x00U, 0x00U},
        .name = "7A cb",
      },
      {
        .mask = operand_mask(operand_type_mask::imm32),
        .operand_format = operand_format::none,
        .opcode_format = opcode_format::plain_2,
        .imm_format = imm_format::rel32,
        .opcode = {0x0FU, 0x8AU, 0x00U, 0x00U},
        .name = "0F 8A cd",
      },
      instruction_encoding{},
    },
  };
  constexpr inline instruction<2> js = {
    .name      = "js",
    .encodings = {
      {
        .mask = operand_mask(operand_type_mask::imm8),
        .operand_format = operand_format::none,
        .opcode_format = opcode_format::plain_1,
        .imm_format = imm_format::rel8,
        .opcode = {0x78U, 0x00U, 0x00U, 0x00U},
        .name = "78 cb",
      },
      {
        .mask = operand_mask(operand_type_mask::imm32),
        .operand_format = operand_format::none,
        .opcode_format = opcode_format::plain_2,
        .imm_format = imm_format::rel32,
        .opcode = {0x0FU, 0x88U, 0x00U, 0x00U},
        .name = "0F 88 cd",
      },
      instruction_encoding{},
    },
  };
}
