#pragma once
// GENERATED HEADER

#include <lisp/assembly/amd64/instruction.hpp>

namespace j::lisp::assembly::amd64::inst {
  constexpr inline instruction<1> seta = {
    .name      = "seta",
    .encodings = {
      {
        .mask = operand_mask(operand_type_mask::rm8),
        .operand_format = operand_format::m,
        .opcode_format = opcode_format::plain_2,
        .opcode = {0x0FU, 0x97U, 0x00U, 0x00U},
        .name = "0F 97 /r",
      },
      instruction_encoding{},
    },
  };
  constexpr inline instruction<1> setc = {
    .name      = "setc",
    .encodings = {
      {
        .mask = operand_mask(operand_type_mask::rm8),
        .operand_format = operand_format::m,
        .opcode_format = opcode_format::plain_2,
        .opcode = {0x0FU, 0x92U, 0x00U, 0x00U},
        .name = "0F 92 /r",
      },
      instruction_encoding{},
    },
  };
  constexpr inline instruction<1> setl = {
    .name      = "setl",
    .encodings = {
      {
        .mask = operand_mask(operand_type_mask::rm8),
        .operand_format = operand_format::m,
        .opcode_format = opcode_format::plain_2,
        .opcode = {0x0FU, 0x9CU, 0x00U, 0x00U},
        .name = "0F 9C /r",
      },
      instruction_encoding{},
    },
  };
  constexpr inline instruction<1> setle = {
    .name      = "setle",
    .encodings = {
      {
        .mask = operand_mask(operand_type_mask::rm8),
        .operand_format = operand_format::m,
        .opcode_format = opcode_format::plain_2,
        .opcode = {0x0FU, 0x9EU, 0x00U, 0x00U},
        .name = "0F 9E /r",
      },
      instruction_encoding{},
    },
  };
  constexpr inline instruction<1> setna = {
    .name      = "setna",
    .encodings = {
      {
        .mask = operand_mask(operand_type_mask::rm8),
        .operand_format = operand_format::m,
        .opcode_format = opcode_format::plain_2,
        .opcode = {0x0FU, 0x96U, 0x00U, 0x00U},
        .name = "0F 96 /r",
      },
      instruction_encoding{},
    },
  };
  constexpr inline instruction<1> setnc = {
    .name      = "setnc",
    .encodings = {
      {
        .mask = operand_mask(operand_type_mask::rm8),
        .operand_format = operand_format::m,
        .opcode_format = opcode_format::plain_2,
        .opcode = {0x0FU, 0x93U, 0x00U, 0x00U},
        .name = "0F 93 /r",
      },
      instruction_encoding{},
    },
  };
  constexpr inline instruction<1> setnl = {
    .name      = "setnl",
    .encodings = {
      {
        .mask = operand_mask(operand_type_mask::rm8),
        .operand_format = operand_format::m,
        .opcode_format = opcode_format::plain_2,
        .opcode = {0x0FU, 0x9DU, 0x00U, 0x00U},
        .name = "0F 9D /r",
      },
      instruction_encoding{},
    },
  };
  constexpr inline instruction<1> setnle = {
    .name      = "setnle",
    .encodings = {
      {
        .mask = operand_mask(operand_type_mask::rm8),
        .operand_format = operand_format::m,
        .opcode_format = opcode_format::plain_2,
        .opcode = {0x0FU, 0x9FU, 0x00U, 0x00U},
        .name = "0F 9F /r",
      },
      instruction_encoding{},
    },
  };
  constexpr inline instruction<1> setno = {
    .name      = "setno",
    .encodings = {
      {
        .mask = operand_mask(operand_type_mask::rm8),
        .operand_format = operand_format::m,
        .opcode_format = opcode_format::plain_2,
        .opcode = {0x0FU, 0x91U, 0x00U, 0x00U},
        .name = "0F 91 /r",
      },
      instruction_encoding{},
    },
  };
  constexpr inline instruction<1> setnp = {
    .name      = "setnp",
    .encodings = {
      {
        .mask = operand_mask(operand_type_mask::rm8),
        .operand_format = operand_format::m,
        .opcode_format = opcode_format::plain_2,
        .opcode = {0x0FU, 0x9BU, 0x00U, 0x00U},
        .name = "0F 9B /r",
      },
      instruction_encoding{},
    },
  };
  constexpr inline instruction<1> setns = {
    .name      = "setns",
    .encodings = {
      {
        .mask = operand_mask(operand_type_mask::rm8),
        .operand_format = operand_format::m,
        .opcode_format = opcode_format::plain_2,
        .opcode = {0x0FU, 0x99U, 0x00U, 0x00U},
        .name = "0F 99 /r",
      },
      instruction_encoding{},
    },
  };
  constexpr inline instruction<1> setnz = {
    .name      = "setnz",
    .encodings = {
      {
        .mask = operand_mask(operand_type_mask::rm8),
        .operand_format = operand_format::m,
        .opcode_format = opcode_format::plain_2,
        .opcode = {0x0FU, 0x95U, 0x00U, 0x00U},
        .name = "0F 95 /r",
      },
      instruction_encoding{},
    },
  };
  constexpr inline instruction<1> seto = {
    .name      = "seto",
    .encodings = {
      {
        .mask = operand_mask(operand_type_mask::rm8),
        .operand_format = operand_format::m,
        .opcode_format = opcode_format::plain_2,
        .opcode = {0x0FU, 0x90U, 0x00U, 0x00U},
        .name = "0F 90 /r",
      },
      instruction_encoding{},
    },
  };
  constexpr inline instruction<1> setp = {
    .name      = "setp",
    .encodings = {
      {
        .mask = operand_mask(operand_type_mask::rm8),
        .operand_format = operand_format::m,
        .opcode_format = opcode_format::plain_2,
        .opcode = {0x0FU, 0x9AU, 0x00U, 0x00U},
        .name = "0F 9A /r",
      },
      instruction_encoding{},
    },
  };
  constexpr inline instruction<1> sets = {
    .name      = "sets",
    .encodings = {
      {
        .mask = operand_mask(operand_type_mask::rm8),
        .operand_format = operand_format::m,
        .opcode_format = opcode_format::plain_2,
        .opcode = {0x0FU, 0x98U, 0x00U, 0x00U},
        .name = "0F 98 /r",
      },
      instruction_encoding{},
    },
  };
  constexpr inline instruction<1> setz = {
    .name      = "setz",
    .encodings = {
      {
        .mask = operand_mask(operand_type_mask::rm8),
        .operand_format = operand_format::m,
        .opcode_format = opcode_format::plain_2,
        .opcode = {0x0FU, 0x94U, 0x00U, 0x00U},
        .name = "0F 94 /r",
      },
      instruction_encoding{},
    },
  };
}
