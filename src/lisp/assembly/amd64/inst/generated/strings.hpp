#pragma once
// GENERATED HEADER

#include <lisp/assembly/amd64/instruction.hpp>

namespace j::lisp::assembly::amd64::inst {
  constexpr inline instruction<1> cld = {
    .name      = "cld",
    .encodings = {
      {
        .mask = operand_mask(),
        .operand_format = operand_format::none,
        .opcode_format = opcode_format::plain_1,
        .opcode = {0xFCU, 0x00U, 0x00U, 0x00U},
        .name = "FC",
      },
      instruction_encoding{},
    },
  };
  constexpr inline instruction<1> lodsb = {
    .name      = "lodsb",
    .encodings = {
      {
        .mask = operand_mask(special_case_al, special_case_any_rsi),
        .operand_format = operand_format::implicit,
        .opcode_format = opcode_format::plain_1,
        .opcode = {0xACU, 0x00U, 0x00U, 0x00U},
        .name = "AC",
      },
      instruction_encoding{},
    },
  };
  constexpr inline instruction<1> lodsd = {
    .name      = "lodsd",
    .encodings = {
      {
        .mask = operand_mask(special_case_eax, special_case_any_rsi),
        .operand_format = operand_format::implicit,
        .opcode_format = opcode_format::plain_1,
        .opcode = {0xADU, 0x00U, 0x00U, 0x00U},
        .name = "AD",
      },
      instruction_encoding{},
    },
  };
  constexpr inline instruction<1> lodsq = {
    .name      = "lodsq",
    .encodings = {
      {
        .mask = operand_mask(special_case_rax, special_case_any_rsi),
        .has_rex_w = true,
        .operand_format = operand_format::implicit,
        .opcode_format = opcode_format::plain_1,
        .opcode = {0xADU, 0x00U, 0x00U, 0x00U},
        .name = "REX.W AD",
      },
      instruction_encoding{},
    },
  };
  constexpr inline instruction<1> lodsw = {
    .name      = "lodsw",
    .encodings = {
      {
        .mask = operand_mask(special_case_ax, special_case_any_rsi),
        .mandatory_prefix = mandatory_prefix::has_66,
        .operand_format = operand_format::implicit,
        .opcode_format = opcode_format::plain_1,
        .opcode = {0xADU, 0x00U, 0x00U, 0x00U},
        .name = "66 AD",
      },
      instruction_encoding{},
    },
  };
  constexpr inline instruction<1> movsb = {
    .name      = "movsb",
    .encodings = {
      {
        .mask = operand_mask(special_case_any_rdi, special_case_any_rsi),
        .operand_format = operand_format::none,
        .opcode_format = opcode_format::plain_1,
        .opcode = {0xA4U, 0x00U, 0x00U, 0x00U},
        .name = "A4",
      },
      instruction_encoding{},
    },
  };
  constexpr inline instruction<4> movsd = {
    .name      = "movsd",
    .encodings = {
      {
        .mask = operand_mask(special_case_any_rdi, special_case_any_rsi),
        .operand_format = operand_format::none,
        .opcode_format = opcode_format::plain_1,
        .opcode = {0xA5U, 0x00U, 0x00U, 0x00U},
        .name = "A5",
      },
      {
        .mask = operand_mask(operand_type_mask::xmm, operand_type_mask::xmm),
        .mandatory_prefix = mandatory_prefix::has_f2,
        .operand_format = operand_format::rm,
        .opcode_format = opcode_format::plain_2,
        .opcode = {0x0FU, 0x10U, 0x00U, 0x00U},
        .name = "F2 0F 10 /r",
      },
      {
        .mask = operand_mask(operand_type_mask::xmm, operand_type_mask::m64),
        .mandatory_prefix = mandatory_prefix::has_f2,
        .operand_format = operand_format::rm,
        .opcode_format = opcode_format::plain_2,
        .opcode = {0x0FU, 0x10U, 0x00U, 0x00U},
        .name = "F2 0F 10 /r",
      },
      {
        .mask = operand_mask(operand_type_mask::m64, operand_type_mask::xmm),
        .mandatory_prefix = mandatory_prefix::has_f2,
        .operand_format = operand_format::mr,
        .opcode_format = opcode_format::plain_2,
        .opcode = {0x0FU, 0x11U, 0x00U, 0x00U},
        .name = "F2 0F 11 /r",
      },
      instruction_encoding{},
    },
  };
  constexpr inline instruction<1> movsq = {
    .name      = "movsq",
    .encodings = {
      {
        .mask = operand_mask(special_case_any_rdi, special_case_any_rsi),
        .has_rex_w = true,
        .operand_format = operand_format::none,
        .opcode_format = opcode_format::plain_1,
        .opcode = {0xA5U, 0x00U, 0x00U, 0x00U},
        .name = "REX.W A5",
      },
      instruction_encoding{},
    },
  };
  constexpr inline instruction<1> movsw = {
    .name      = "movsw",
    .encodings = {
      {
        .mask = operand_mask(special_case_any_rdi, special_case_any_rsi),
        .mandatory_prefix = mandatory_prefix::has_66,
        .operand_format = operand_format::none,
        .opcode_format = opcode_format::plain_1,
        .opcode = {0xA5U, 0x00U, 0x00U, 0x00U},
        .name = "66 A5",
      },
      instruction_encoding{},
    },
  };
  constexpr inline instruction<1> std = {
    .name      = "std",
    .encodings = {
      {
        .mask = operand_mask(),
        .operand_format = operand_format::none,
        .opcode_format = opcode_format::plain_1,
        .opcode = {0xFDU, 0x00U, 0x00U, 0x00U},
        .name = "FD",
      },
      instruction_encoding{},
    },
  };
  constexpr inline instruction<1> stosb = {
    .name      = "stosb",
    .encodings = {
      {
        .mask = operand_mask(special_case_any_rdi, special_case_al),
        .operand_format = operand_format::implicit,
        .opcode_format = opcode_format::plain_1,
        .opcode = {0xAAU, 0x00U, 0x00U, 0x00U},
        .name = "AA",
      },
      instruction_encoding{},
    },
  };
  constexpr inline instruction<1> stosd = {
    .name      = "stosd",
    .encodings = {
      {
        .mask = operand_mask(special_case_any_rdi, special_case_eax),
        .operand_format = operand_format::implicit,
        .opcode_format = opcode_format::plain_1,
        .opcode = {0xABU, 0x00U, 0x00U, 0x00U},
        .name = "AB",
      },
      instruction_encoding{},
    },
  };
  constexpr inline instruction<1> stosq = {
    .name      = "stosq",
    .encodings = {
      {
        .mask = operand_mask(special_case_any_rdi, special_case_rax),
        .has_rex_w = true,
        .operand_format = operand_format::implicit,
        .opcode_format = opcode_format::plain_1,
        .opcode = {0xABU, 0x00U, 0x00U, 0x00U},
        .name = "REX.W AB",
      },
      instruction_encoding{},
    },
  };
  constexpr inline instruction<1> stosw = {
    .name      = "stosw",
    .encodings = {
      {
        .mask = operand_mask(special_case_any_rdi, special_case_ax),
        .mandatory_prefix = mandatory_prefix::has_66,
        .operand_format = operand_format::implicit,
        .opcode_format = opcode_format::plain_1,
        .opcode = {0xABU, 0x00U, 0x00U, 0x00U},
        .name = "66 AB",
      },
      instruction_encoding{},
    },
  };
}
