#pragma once

#include "strings/formatting/formatter.hpp"

namespace j::mem {
  class memory_mapping_info;
}

namespace j::mem::debug {
  extern const strings::formatter_known_length<memory_mapping_info> & g_memory_mapping_info_formatter;
}
