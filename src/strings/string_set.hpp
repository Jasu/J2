#pragma once

#include "strings/string_map_fwd.hpp"

J_DECLARE_EXTERN_STRING_MAP(void);

namespace j::strings {
  using string_set = containers::hash_set<strings::string, string_hash>;
}
