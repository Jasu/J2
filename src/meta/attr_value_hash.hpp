#pragma once

#include "meta/attr_value.hpp"

namespace j::meta {
  struct attr_value_hash final {
    [[nodiscard]] u32_t operator()(const attr_value & v) const noexcept;
  };
}
