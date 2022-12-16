#include "properties/access/bool_access.hpp"
#include "exceptions/assert_lite.hpp"

namespace j::properties::access {
  bool_access_definition::bool_access_definition() noexcept {
  }

  bool_access_definition::~bool_access_definition() {
  }

  void bool_access_definition::initialize_bool_access(get_value_t get_value, set_value_t set_value) {
    m_get_value = get_value;
    m_set_value = set_value;
    J_ASSERT_NOT_NULL(m_get_value, m_set_value);
  }
}
