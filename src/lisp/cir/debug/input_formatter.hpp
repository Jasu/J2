#pragma once

#include "strings/formatting/formatter.hpp"

namespace j::lisp::cir::inline ops {
  struct op_result_input;
  struct reloc_input;
  struct mem_input;
  struct atomic_input;
  struct input;
}

namespace j::lisp::cir::inline debug {
  extern const strings::formatter_known_length<op_result_input> & g_op_result_type_formatter;
  extern const strings::formatter_known_length<reloc_input> & g_op_reloc_input_formatter;
  extern const strings::formatter_known_length<atomic_input> & g_atomic_input_formatter;
  extern const strings::formatter_known_length<mem_input> & g_mem_input_formatter;
  extern const strings::formatter_known_length<input> & g_input_formatter;
}
