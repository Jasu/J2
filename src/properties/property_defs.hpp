#pragma once

#include "properties/property_type.hpp"
#include "containers/unsorted_string_map_fwd.hpp"
#include "containers/vector.hpp"
#include "hzd/types.hpp"

namespace j::properties {
  class property_variant;
}

namespace j::properties {
  using nil_t J_NO_DEBUG_TYPE = null_t;
  using string_t J_NO_DEBUG_TYPE = strings::string;
  using int_t J_NO_DEBUG_TYPE = i64_t;
  using bool_t J_NO_DEBUG_TYPE = bool;
  using float_t J_NO_DEBUG_TYPE = double;
  using list_t J_NO_DEBUG_TYPE = vector<property_variant>;
  using map_t J_NO_DEBUG_TYPE = unsorted_string_map<property_variant, true>;
}
