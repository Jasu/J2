#pragma once

#include "hzd/utility.hpp"

namespace j::attributes {
  /// Wrapper object for named argument passed to a function.
  template<typename AttributeDefinition, typename Value>
  class attribute final {
  public:
    using tag_t J_NO_DEBUG_TYPE = typename AttributeDefinition::tag_t;
    Value m_value;
  };

  template<typename AttributeDefinition, typename T>
  class attribute<AttributeDefinition, type_tag<T>> final {
  public:
    using tag_t J_NO_DEBUG_TYPE = typename AttributeDefinition::tag_t;
    using type = J_NO_DEBUG_TYPE T;
  };
}
