#pragma once

#include "rendering/data_types/data_type.hpp"

namespace j::rendering::vulkan {
  inline namespace uniform_buffers {
    struct J_PACKED uniform_buffer_layout final {
      data_types::data_type types[12] = { data_types::data_type::none };
      i16_t size = -1;
    };
    static_assert(sizeof(uniform_buffer_layout) == 14);
  }
}
