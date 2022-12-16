#pragma once

#include "hzd/utility.hpp"

namespace j::rendering::vulkan::inline spirv {
  struct J_PACKED spirv_header final {
    u32_t magic;
    u32_t version;
    u32_t generator_magic;
    u32_t id_bound;
    u32_t reserved;
  };
  static_assert(sizeof(spirv_header) == 4 * 5);
}
