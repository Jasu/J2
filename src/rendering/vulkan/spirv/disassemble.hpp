#pragma once

#include "hzd/types.hpp"

namespace j::rendering::vulkan::inline spirv {
  void disassemble(const u8_t * spirv, u32_t size);
}
