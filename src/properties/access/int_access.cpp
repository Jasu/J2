#include "properties/access/int_access.hpp"
#include "exceptions/assert_lite.hpp"

namespace j::properties::access {
  int_access_definition::int_access_definition() noexcept {
  }

  int_access_definition::~int_access_definition() {
  }

  void int_access_definition::initialize_int_access(
    int_access_definition::get_value_t get_value,
    int_access_definition::set_value_t set_value
  ) {
    m_get_value = get_value;
    m_set_value = set_value;

    J_ASSERT_NOT_NULL(get_value, set_value);
  }
}
