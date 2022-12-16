#pragma once

#include "hzd/utility.hpp"
#include "strings/string_view.hpp"
#include "lisp/common/mem_width.hpp"

namespace lisp::asm_parser {
  enum class operand_type : u8_t {
    none,

    imm,
    mem,
    reg,
  };

  struct reg_data final {
    j::lisp::mem_width width = j::lisp::mem_width::none;
    j::strings::const_string_view mnemonic;
  };

  struct operand final {
    operand_type type = operand_type::none;
    union {
      reg_data reg_data;
    };
  };

}
