#pragma once

#include "strings/formatting/formatter.hpp"
#include "lisp/assembly/registers.hpp"

namespace j::lisp::assembly {
  extern const strings::formatter_known_length<phys_reg> & g_preg_formatter;
  extern const strings::formatter_known_length<reg> & g_reg_formatter;
}
