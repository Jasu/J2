#pragma once

#include "lisp/common/mem_scale.hpp"
#include "lisp/common/truthiness.hpp"
#include "strings/formatting/formatter.hpp"

namespace j::lisp::inline common {
  extern const strings::formatter_known_length<mem_width> & g_mem_width_formatter;
  extern const strings::formatter_known_length<mem_scale> & g_mem_scale_formatter;
  extern const strings::formatter_known_length<truthiness> & g_truthiness_formatter;
}
