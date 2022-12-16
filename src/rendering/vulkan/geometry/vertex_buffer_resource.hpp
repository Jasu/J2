#pragma once

#include "rendering/vulkan/resources/resource_declaration.hpp"

namespace j::rendering::vulkan::geometry {
  class host_visible_vertex_buffer;
  struct vertex_buffer_description;
  struct vertex_buffer_state;
  using vertex_buffer = host_visible_vertex_buffer;
  DECLARE_RESOURCE(vertex_buffer, vertex_buffer_description, vertex_buffer_state);
}
DECLARE_RESOURCE_TEMPLATES(j::rendering::vulkan::geometry::vertex_buffer, j::rendering::vulkan::geometry::vertex_buffer_description);
