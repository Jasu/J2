#pragma once

#include "hzd/utility.hpp"

namespace j::geometry {
  enum class axis : u8_t {
    vertical,
    horizontal,
  };

  J_INLINE_GETTER constexpr axis perpendicular(axis a) noexcept {
    return a == axis::vertical ? axis::horizontal : axis::vertical;
  }
}
