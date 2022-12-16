#pragma once

#include "hzd/utility.hpp"

namespace j::mem {
  class any;
}

namespace j::attributes {
  namespace detail {
    struct attribute_tag_tag;
    struct attribute_key_tag;
    struct attribute_value_type_tag;
  }

  template<typename AttrDef, typename ValueType> class attribute;

  template<typename ValueType, typename Tag, typename Policy, typename KeyType>
  struct attribute_definition;

  using any J_NO_DEBUG_TYPE = mem::any;

  struct type_value;

  struct flag;

  /// Policy that allows the attribute to be specified only once.
  struct single;

  /// Policy that allows the attribute to be specified multiple times.
  struct multiple;
}
