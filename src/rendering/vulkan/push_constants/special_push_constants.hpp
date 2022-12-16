#pragma once

#include "hzd/types.hpp"

namespace j::rendering::vulkan {
  inline namespace push_constants {
  enum class special_push_constant : u8_t {
    /// Push the size of the current viewport, in pixels, as a float vector.
    viewport_size_vec2,
    /// Push the inverse size of the current viewport, in pixels, as a float vector.
    inverse_viewport_size_vec2,
    /// Sentinel, do not use.
    max,
  };
  }
}
