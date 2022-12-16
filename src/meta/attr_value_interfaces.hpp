#pragma once

#include "meta/attr_value_interface.hpp"

namespace j::meta {
  extern const attr_value_interface*const interfaces[num_attr_types_v + 1U];

  J_A(AI,ND,NODISC) inline const attr_value_interface & interface_of(attr_type t) noexcept {
    return *interfaces[t];
  }

  J_A(AI,ND,NODISC) inline const attr_value_interface & interface_of(const attr_value & v) noexcept {
    return *interfaces[v.type];
  }
}
