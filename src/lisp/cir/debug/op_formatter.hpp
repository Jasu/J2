#pragma once

#include "lisp/cir/ops/op_type.hpp"
#include "lisp/cir/ops/op_index.hpp"
#include "strings/formatting/formatter.hpp"

namespace j::lisp::cir::inline debug {
  extern const strings::formatter_known_length<op_type> & g_op_type_formatter;
  extern const strings::formatter_known_length<op_index> & g_op_index_formatter;
}
