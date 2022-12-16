#pragma once

#include "properties/wrappers/wrapper.hpp"

namespace j::properties {
  class property_variant;
}

namespace j::properties::wrappers {
  namespace detail {
    extern const wrapper_definition variant_wrapper_definition;
  }

  inline wrapper access(property_variant & variant) noexcept {
    return {detail::variant_wrapper_definition, &variant};
  }
}
