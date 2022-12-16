#pragma once

#include "strings/formatting/formatter.hpp"

namespace j::lisp::air::inline values {
  struct val_spec;
}
namespace j::lisp::air {
  namespace exprs {
    struct expr;
  }

  enum class expr_type : u8_t;
  extern const strings::formatter_known_length<expr_type> & g_expr_type_formatter;
  extern const strings::formatter_known_length<val_spec> & g_val_spec_formatter;
  inline namespace debug {
    extern const strings::formatter_known_length<exprs::expr> & g_expr_formatter;
  }
}
