#pragma once

#include "strings/formatting/formatter.hpp"

namespace j::strings::formatters {
  extern constinit const formatter_known_length<const void*, void*> & g_void_star_formatter;
}
