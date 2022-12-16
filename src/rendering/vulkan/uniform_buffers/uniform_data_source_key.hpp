#pragma once

#include "rendering/data_sources/key.hpp"

namespace j::rendering::vulkan {
  inline namespace uniform_buffers {
    using uniform_data_source_key = data_sources::key<struct uniform_source_tag>;
  }
}
