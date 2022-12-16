#pragma once

#include "rendering/vulkan/buffers/suballocated_buffer.hpp"
#include "rendering/vulkan/transfer/transfer_source_buffer_resource.hpp"

namespace j::rendering::vulkan::transfer {
  class transfer_source_buffer final : public buffers::suballocated_buffer<transfer_source_buffer_ref> {
  public:
    transfer_source_buffer(const device_context_base & context, u32_t size);
  };
}
