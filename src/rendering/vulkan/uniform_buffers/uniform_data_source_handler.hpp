#pragma once

#include "rendering/vulkan/uniform_buffers/uniform_data_source_key.hpp"
#include "rendering/data_sources/source_handler.hpp"

namespace j::rendering::vulkan {
  inline namespace uniform_buffers {
    struct uniform_buffer_layout;
    namespace detail {
      template<typename> struct uniform_view;
    }
    using uniform_view = detail::uniform_view<u8_t>;
    using uniform_data_source_handler
      = data_sources::source_handler<uniform_data_source_key, uniform_buffer_layout, uniform_view>;
  }
}
