#pragma once

#include "lisp/common/id.hpp"
#include "strings/formatting/formatter.hpp"

namespace j::lisp::env {
  extern constinit const strings::formatter_known_length<id> & g_id_formatter;
}
