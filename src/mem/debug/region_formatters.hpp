#pragma once

#include "strings/formatting/formatter.hpp"
#include "mem/memory_region.hpp"

namespace j::mem::debug {
  extern const strings::formatter_known_length<memory_region> & g_memory_region_formatter;
  extern const strings::formatter_known_length<const_memory_region> & g_const_memory_region_formatter;
}
