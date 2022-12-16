#pragma once

#include "properties/wrappers/wrapper.hpp"
#include "properties/access/registry.hpp"

namespace j::properties::wrappers {
  template<typename T>
  wrapper access(T & ref) noexcept {
    return {*access::registry::get_wrapper_definition<T>(), &ref};
  }
}
