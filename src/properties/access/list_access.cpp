#include "properties/access/list_access.hpp"
#include "properties/detail/value_definition_impl.hpp"

namespace j::properties::access {
  void list_access_definition::set_item_definition(
    const wrappers::wrapper_definition * item_definition
  ) {
    J_ASSERT(!m_item_definition, "list_access_definition::set_item_definition was called twice.");
    J_ASSERT_NOT_NULL(item_definition);
    m_item_definition = item_definition;
  }
}
