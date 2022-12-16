#pragma once

#include "properties/access/registry.hpp"

namespace j::properties::wrappers {
  class wrapper_definition;

  template<typename Wrapper>
  class J_TYPE_HIDDEN wrapper_registration {
  public:
    explicit wrapper_registration(const wrapper_definition * wrapper) {
      access::registry::add_wrapper(typeid(Wrapper), wrapper);
    }

    ~wrapper_registration() {
      access::registry::remove_wrapper(typeid(Wrapper));
    }
  };
}
