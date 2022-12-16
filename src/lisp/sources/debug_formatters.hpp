#pragma once

#include "strings/formatting/formatter.hpp"
#include "lisp/sources/source_location.hpp"

namespace j::lisp::sources {
  extern const strings::formatter_known_length<source_location> & g_source_location_formatter;
}
