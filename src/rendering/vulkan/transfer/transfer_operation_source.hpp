#pragma once

#include "rendering/vulkan/transfer/transfer_source_buffer_resource.hpp"
#include "rendering/vulkan/buffers/suballocation.hpp"

namespace j::rendering::vulkan::transfer {
  using transfer_operation_source = buffers::suballocation<transfer_source_buffer_ref>;
}
