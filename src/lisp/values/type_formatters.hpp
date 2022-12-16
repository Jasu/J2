#pragma once

#include "lisp/values/imm_type.hpp"
#include "strings/formatting/formatter.hpp"

namespace j::lisp::inline values {
  extern const strings::formatter_known_length<imm_type> & g_imm_type_formatter;
  extern const strings::formatter_known_length<imm_type_mask> & g_imm_type_mask_formatter;
}
