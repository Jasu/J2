#pragma once

#include "rendering/vulkan/geometry/vertex_buffer_resource.hpp"
#include "rendering/vulkan/buffers/suballocation.hpp"

namespace j::rendering::vulkan::geometry {
  using vertex_buffer_suballocation = buffers::suballocation<vertex_buffer_ref>;
}
