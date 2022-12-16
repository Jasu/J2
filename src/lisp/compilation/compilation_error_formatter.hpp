#include "strings/formatting/formatter.hpp"
#include "lisp/compilation/compilation_errors.hpp"

namespace j::lisp::compilation {
  extern const strings::formatter_known_length<compilation_error> & g_compilation_error_formatter;
}
