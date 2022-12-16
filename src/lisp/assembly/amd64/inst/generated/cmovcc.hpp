#pragma once
// GENERATED HEADER

#include <lisp/assembly/amd64/instruction.hpp>

namespace j::lisp::assembly::amd64::inst {
  constexpr inline instruction<3> cmova = {
    .name      = "cmova",
    .encodings = {
      {
        .mask = operand_mask(operand_type_mask::r32, operand_type_mask::rm32),
        .operand_format = operand_format::rm,
        .opcode_format = opcode_format::plain_2,
        .opcode = {0x0FU, 0x47U, 0x00U, 0x00U},
        .name = "0F 47 /r",
      },
      {
        .mask = operand_mask(operand_type_mask::r64, operand_type_mask::rm64),
        .has_rex_w = true,
        .operand_format = operand_format::rm,
        .opcode_format = opcode_format::plain_2,
        .opcode = {0x0FU, 0x47U, 0x00U, 0x00U},
        .name = "REX.W 0F 47 /r",
      },
      {
        .mask = operand_mask(operand_type_mask::r16, operand_type_mask::rm16),
        .mandatory_prefix = mandatory_prefix::has_66,
        .operand_format = operand_format::rm,
        .opcode_format = opcode_format::plain_2,
        .opcode = {0x0FU, 0x47U, 0x00U, 0x00U},
        .name = "66 0F 47 /r",
      },
      instruction_encoding{},
    },
  };
  constexpr inline instruction<3> cmovc = {
    .name      = "cmovc",
    .encodings = {
      {
        .mask = operand_mask(operand_type_mask::r32, operand_type_mask::rm32),
        .operand_format = operand_format::rm,
        .opcode_format = opcode_format::plain_2,
        .opcode = {0x0FU, 0x42U, 0x00U, 0x00U},
        .name = "0F 42 /r",
      },
      {
        .mask = operand_mask(operand_type_mask::r64, operand_type_mask::rm64),
        .has_rex_w = true,
        .operand_format = operand_format::rm,
        .opcode_format = opcode_format::plain_2,
        .opcode = {0x0FU, 0x42U, 0x00U, 0x00U},
        .name = "REX.W 0F 42 /r",
      },
      {
        .mask = operand_mask(operand_type_mask::r16, operand_type_mask::rm16),
        .mandatory_prefix = mandatory_prefix::has_66,
        .operand_format = operand_format::rm,
        .opcode_format = opcode_format::plain_2,
        .opcode = {0x0FU, 0x42U, 0x00U, 0x00U},
        .name = "66 0F 42 /r",
      },
      instruction_encoding{},
    },
  };
  constexpr inline instruction<3> cmovl = {
    .name      = "cmovl",
    .encodings = {
      {
        .mask = operand_mask(operand_type_mask::r32, operand_type_mask::rm32),
        .operand_format = operand_format::rm,
        .opcode_format = opcode_format::plain_2,
        .opcode = {0x0FU, 0x4CU, 0x00U, 0x00U},
        .name = "0F 4C /r",
      },
      {
        .mask = operand_mask(operand_type_mask::r64, operand_type_mask::rm64),
        .has_rex_w = true,
        .operand_format = operand_format::rm,
        .opcode_format = opcode_format::plain_2,
        .opcode = {0x0FU, 0x4CU, 0x00U, 0x00U},
        .name = "REX.W 0F 4C /r",
      },
      {
        .mask = operand_mask(operand_type_mask::r16, operand_type_mask::rm16),
        .mandatory_prefix = mandatory_prefix::has_66,
        .operand_format = operand_format::rm,
        .opcode_format = opcode_format::plain_2,
        .opcode = {0x0FU, 0x4CU, 0x00U, 0x00U},
        .name = "66 0F 4C /r",
      },
      instruction_encoding{},
    },
  };
  constexpr inline instruction<3> cmovle = {
    .name      = "cmovle",
    .encodings = {
      {
        .mask = operand_mask(operand_type_mask::r32, operand_type_mask::rm32),
        .operand_format = operand_format::rm,
        .opcode_format = opcode_format::plain_2,
        .opcode = {0x0FU, 0x4EU, 0x00U, 0x00U},
        .name = "0F 4E /r",
      },
      {
        .mask = operand_mask(operand_type_mask::r64, operand_type_mask::rm64),
        .has_rex_w = true,
        .operand_format = operand_format::rm,
        .opcode_format = opcode_format::plain_2,
        .opcode = {0x0FU, 0x4EU, 0x00U, 0x00U},
        .name = "REX.W 0F 4E /r",
      },
      {
        .mask = operand_mask(operand_type_mask::r16, operand_type_mask::rm16),
        .mandatory_prefix = mandatory_prefix::has_66,
        .operand_format = operand_format::rm,
        .opcode_format = opcode_format::plain_2,
        .opcode = {0x0FU, 0x4EU, 0x00U, 0x00U},
        .name = "66 0F 4E /r",
      },
      instruction_encoding{},
    },
  };
  constexpr inline instruction<3> cmovna = {
    .name      = "cmovna",
    .encodings = {
      {
        .mask = operand_mask(operand_type_mask::r32, operand_type_mask::rm32),
        .operand_format = operand_format::rm,
        .opcode_format = opcode_format::plain_2,
        .opcode = {0x0FU, 0x46U, 0x00U, 0x00U},
        .name = "0F 46 /r",
      },
      {
        .mask = operand_mask(operand_type_mask::r64, operand_type_mask::rm64),
        .has_rex_w = true,
        .operand_format = operand_format::rm,
        .opcode_format = opcode_format::plain_2,
        .opcode = {0x0FU, 0x46U, 0x00U, 0x00U},
        .name = "REX.W 0F 46 /r",
      },
      {
        .mask = operand_mask(operand_type_mask::r16, operand_type_mask::rm16),
        .mandatory_prefix = mandatory_prefix::has_66,
        .operand_format = operand_format::rm,
        .opcode_format = opcode_format::plain_2,
        .opcode = {0x0FU, 0x46U, 0x00U, 0x00U},
        .name = "66 0F 46 /r",
      },
      instruction_encoding{},
    },
  };
  constexpr inline instruction<3> cmovnc = {
    .name      = "cmovnc",
    .encodings = {
      {
        .mask = operand_mask(operand_type_mask::r32, operand_type_mask::rm32),
        .operand_format = operand_format::rm,
        .opcode_format = opcode_format::plain_2,
        .opcode = {0x0FU, 0x43U, 0x00U, 0x00U},
        .name = "0F 43 /r",
      },
      {
        .mask = operand_mask(operand_type_mask::r64, operand_type_mask::rm64),
        .has_rex_w = true,
        .operand_format = operand_format::rm,
        .opcode_format = opcode_format::plain_2,
        .opcode = {0x0FU, 0x43U, 0x00U, 0x00U},
        .name = "REX.W 0F 43 /r",
      },
      {
        .mask = operand_mask(operand_type_mask::r16, operand_type_mask::rm16),
        .mandatory_prefix = mandatory_prefix::has_66,
        .operand_format = operand_format::rm,
        .opcode_format = opcode_format::plain_2,
        .opcode = {0x0FU, 0x43U, 0x00U, 0x00U},
        .name = "66 0F 43 /r",
      },
      instruction_encoding{},
    },
  };
  constexpr inline instruction<3> cmovnl = {
    .name      = "cmovnl",
    .encodings = {
      {
        .mask = operand_mask(operand_type_mask::r32, operand_type_mask::rm32),
        .operand_format = operand_format::rm,
        .opcode_format = opcode_format::plain_2,
        .opcode = {0x0FU, 0x4DU, 0x00U, 0x00U},
        .name = "0F 4D /r",
      },
      {
        .mask = operand_mask(operand_type_mask::r64, operand_type_mask::rm64),
        .has_rex_w = true,
        .operand_format = operand_format::rm,
        .opcode_format = opcode_format::plain_2,
        .opcode = {0x0FU, 0x4DU, 0x00U, 0x00U},
        .name = "REX.W 0F 4D /r",
      },
      {
        .mask = operand_mask(operand_type_mask::r16, operand_type_mask::rm16),
        .mandatory_prefix = mandatory_prefix::has_66,
        .operand_format = operand_format::rm,
        .opcode_format = opcode_format::plain_2,
        .opcode = {0x0FU, 0x4DU, 0x00U, 0x00U},
        .name = "66 0F 4D /r",
      },
      instruction_encoding{},
    },
  };
  constexpr inline instruction<3> cmovnle = {
    .name      = "cmovnle",
    .encodings = {
      {
        .mask = operand_mask(operand_type_mask::r32, operand_type_mask::rm32),
        .operand_format = operand_format::rm,
        .opcode_format = opcode_format::plain_2,
        .opcode = {0x0FU, 0x4FU, 0x00U, 0x00U},
        .name = "0F 4F /r",
      },
      {
        .mask = operand_mask(operand_type_mask::r64, operand_type_mask::rm64),
        .has_rex_w = true,
        .operand_format = operand_format::rm,
        .opcode_format = opcode_format::plain_2,
        .opcode = {0x0FU, 0x4FU, 0x00U, 0x00U},
        .name = "REX.W 0F 4F /r",
      },
      {
        .mask = operand_mask(operand_type_mask::r16, operand_type_mask::rm16),
        .mandatory_prefix = mandatory_prefix::has_66,
        .operand_format = operand_format::rm,
        .opcode_format = opcode_format::plain_2,
        .opcode = {0x0FU, 0x4FU, 0x00U, 0x00U},
        .name = "66 0F 4F /r",
      },
      instruction_encoding{},
    },
  };
  constexpr inline instruction<3> cmovno = {
    .name      = "cmovno",
    .encodings = {
      {
        .mask = operand_mask(operand_type_mask::r32, operand_type_mask::rm32),
        .operand_format = operand_format::rm,
        .opcode_format = opcode_format::plain_2,
        .opcode = {0x0FU, 0x41U, 0x00U, 0x00U},
        .name = "0F 41 /r",
      },
      {
        .mask = operand_mask(operand_type_mask::r64, operand_type_mask::rm64),
        .has_rex_w = true,
        .operand_format = operand_format::rm,
        .opcode_format = opcode_format::plain_2,
        .opcode = {0x0FU, 0x41U, 0x00U, 0x00U},
        .name = "REX.W 0F 41 /r",
      },
      {
        .mask = operand_mask(operand_type_mask::r16, operand_type_mask::rm16),
        .mandatory_prefix = mandatory_prefix::has_66,
        .operand_format = operand_format::rm,
        .opcode_format = opcode_format::plain_2,
        .opcode = {0x0FU, 0x41U, 0x00U, 0x00U},
        .name = "66 0F 41 /r",
      },
      instruction_encoding{},
    },
  };
  constexpr inline instruction<3> cmovnp = {
    .name      = "cmovnp",
    .encodings = {
      {
        .mask = operand_mask(operand_type_mask::r32, operand_type_mask::rm32),
        .operand_format = operand_format::rm,
        .opcode_format = opcode_format::plain_2,
        .opcode = {0x0FU, 0x4BU, 0x00U, 0x00U},
        .name = "0F 4B /r",
      },
      {
        .mask = operand_mask(operand_type_mask::r64, operand_type_mask::rm64),
        .has_rex_w = true,
        .operand_format = operand_format::rm,
        .opcode_format = opcode_format::plain_2,
        .opcode = {0x0FU, 0x4BU, 0x00U, 0x00U},
        .name = "REX.W 0F 4B /r",
      },
      {
        .mask = operand_mask(operand_type_mask::r16, operand_type_mask::rm16),
        .mandatory_prefix = mandatory_prefix::has_66,
        .operand_format = operand_format::rm,
        .opcode_format = opcode_format::plain_2,
        .opcode = {0x0FU, 0x4BU, 0x00U, 0x00U},
        .name = "66 0F 4B /r",
      },
      instruction_encoding{},
    },
  };
  constexpr inline instruction<3> cmovns = {
    .name      = "cmovns",
    .encodings = {
      {
        .mask = operand_mask(operand_type_mask::r32, operand_type_mask::rm32),
        .operand_format = operand_format::rm,
        .opcode_format = opcode_format::plain_2,
        .opcode = {0x0FU, 0x49U, 0x00U, 0x00U},
        .name = "0F 49 /r",
      },
      {
        .mask = operand_mask(operand_type_mask::r64, operand_type_mask::rm64),
        .has_rex_w = true,
        .operand_format = operand_format::rm,
        .opcode_format = opcode_format::plain_2,
        .opcode = {0x0FU, 0x49U, 0x00U, 0x00U},
        .name = "REX.W 0F 49 /r",
      },
      {
        .mask = operand_mask(operand_type_mask::r16, operand_type_mask::rm16),
        .mandatory_prefix = mandatory_prefix::has_66,
        .operand_format = operand_format::rm,
        .opcode_format = opcode_format::plain_2,
        .opcode = {0x0FU, 0x49U, 0x00U, 0x00U},
        .name = "66 0F 49 /r",
      },
      instruction_encoding{},
    },
  };
  constexpr inline instruction<3> cmovnz = {
    .name      = "cmovnz",
    .encodings = {
      {
        .mask = operand_mask(operand_type_mask::r32, operand_type_mask::rm32),
        .operand_format = operand_format::rm,
        .opcode_format = opcode_format::plain_2,
        .opcode = {0x0FU, 0x45U, 0x00U, 0x00U},
        .name = "0F 45 /r",
      },
      {
        .mask = operand_mask(operand_type_mask::r64, operand_type_mask::rm64),
        .has_rex_w = true,
        .operand_format = operand_format::rm,
        .opcode_format = opcode_format::plain_2,
        .opcode = {0x0FU, 0x45U, 0x00U, 0x00U},
        .name = "REX.W 0F 45 /r",
      },
      {
        .mask = operand_mask(operand_type_mask::r16, operand_type_mask::rm16),
        .mandatory_prefix = mandatory_prefix::has_66,
        .operand_format = operand_format::rm,
        .opcode_format = opcode_format::plain_2,
        .opcode = {0x0FU, 0x45U, 0x00U, 0x00U},
        .name = "66 0F 45 /r",
      },
      instruction_encoding{},
    },
  };
  constexpr inline instruction<3> cmovo = {
    .name      = "cmovo",
    .encodings = {
      {
        .mask = operand_mask(operand_type_mask::r32, operand_type_mask::rm32),
        .operand_format = operand_format::rm,
        .opcode_format = opcode_format::plain_2,
        .opcode = {0x0FU, 0x40U, 0x00U, 0x00U},
        .name = "0F 40 /r",
      },
      {
        .mask = operand_mask(operand_type_mask::r64, operand_type_mask::rm64),
        .has_rex_w = true,
        .operand_format = operand_format::rm,
        .opcode_format = opcode_format::plain_2,
        .opcode = {0x0FU, 0x40U, 0x00U, 0x00U},
        .name = "REX.W 0F 40 /r",
      },
      {
        .mask = operand_mask(operand_type_mask::r16, operand_type_mask::rm16),
        .mandatory_prefix = mandatory_prefix::has_66,
        .operand_format = operand_format::rm,
        .opcode_format = opcode_format::plain_2,
        .opcode = {0x0FU, 0x40U, 0x00U, 0x00U},
        .name = "66 0F 40 /r",
      },
      instruction_encoding{},
    },
  };
  constexpr inline instruction<3> cmovp = {
    .name      = "cmovp",
    .encodings = {
      {
        .mask = operand_mask(operand_type_mask::r32, operand_type_mask::rm32),
        .operand_format = operand_format::rm,
        .opcode_format = opcode_format::plain_2,
        .opcode = {0x0FU, 0x4AU, 0x00U, 0x00U},
        .name = "0F 4A /r",
      },
      {
        .mask = operand_mask(operand_type_mask::r64, operand_type_mask::rm64),
        .has_rex_w = true,
        .operand_format = operand_format::rm,
        .opcode_format = opcode_format::plain_2,
        .opcode = {0x0FU, 0x4AU, 0x00U, 0x00U},
        .name = "REX.W 0F 4A /r",
      },
      {
        .mask = operand_mask(operand_type_mask::r16, operand_type_mask::rm16),
        .mandatory_prefix = mandatory_prefix::has_66,
        .operand_format = operand_format::rm,
        .opcode_format = opcode_format::plain_2,
        .opcode = {0x0FU, 0x4AU, 0x00U, 0x00U},
        .name = "66 0F 4A /r",
      },
      instruction_encoding{},
    },
  };
  constexpr inline instruction<3> cmovs = {
    .name      = "cmovs",
    .encodings = {
      {
        .mask = operand_mask(operand_type_mask::r32, operand_type_mask::rm32),
        .operand_format = operand_format::rm,
        .opcode_format = opcode_format::plain_2,
        .opcode = {0x0FU, 0x48U, 0x00U, 0x00U},
        .name = "0F 48 /r",
      },
      {
        .mask = operand_mask(operand_type_mask::r64, operand_type_mask::rm64),
        .has_rex_w = true,
        .operand_format = operand_format::rm,
        .opcode_format = opcode_format::plain_2,
        .opcode = {0x0FU, 0x48U, 0x00U, 0x00U},
        .name = "REX.W 0F 48 /r",
      },
      {
        .mask = operand_mask(operand_type_mask::r16, operand_type_mask::rm16),
        .mandatory_prefix = mandatory_prefix::has_66,
        .operand_format = operand_format::rm,
        .opcode_format = opcode_format::plain_2,
        .opcode = {0x0FU, 0x48U, 0x00U, 0x00U},
        .name = "66 0F 48 /r",
      },
      instruction_encoding{},
    },
  };
  constexpr inline instruction<3> cmovz = {
    .name      = "cmovz",
    .encodings = {
      {
        .mask = operand_mask(operand_type_mask::r32, operand_type_mask::rm32),
        .operand_format = operand_format::rm,
        .opcode_format = opcode_format::plain_2,
        .opcode = {0x0FU, 0x44U, 0x00U, 0x00U},
        .name = "0F 44 /r",
      },
      {
        .mask = operand_mask(operand_type_mask::r64, operand_type_mask::rm64),
        .has_rex_w = true,
        .operand_format = operand_format::rm,
        .opcode_format = opcode_format::plain_2,
        .opcode = {0x0FU, 0x44U, 0x00U, 0x00U},
        .name = "REX.W 0F 44 /r",
      },
      {
        .mask = operand_mask(operand_type_mask::r16, operand_type_mask::rm16),
        .mandatory_prefix = mandatory_prefix::has_66,
        .operand_format = operand_format::rm,
        .opcode_format = opcode_format::plain_2,
        .opcode = {0x0FU, 0x44U, 0x00U, 0x00U},
        .name = "66 0F 44 /r",
      },
      instruction_encoding{},
    },
  };
}
