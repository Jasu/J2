#include "properties/access/enum_access.hpp"
#include "containers/unsorted_string_map.hpp"

namespace j::properties::access {
  void enum_access_definition::initialize_enum_access(get_value_t get_value, set_value_t set_value) {
    m_get_value = get_value, m_set_value = set_value;
    J_ASSERT_NOT_NULL(m_get_value, m_set_value);
  }
}
