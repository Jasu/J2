#include "rendering/vulkan/geometry/vertex_buffer.hpp"

#include "rendering/vulkan/geometry/vertex_buffer_description.hpp"
#include "rendering/vulkan/geometry/vertex_buffer_state.hpp"
#include "rendering/vulkan/resources/resource_definition.hpp"

namespace j::rendering::vulkan::geometry {
  host_visible_vertex_buffer::host_visible_vertex_buffer(
    const device_context_base & context,
    const vertex_buffer_description & description)
  : buffers::suballocated_buffer<vertex_buffer_ref>(
      context,
      VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
      description.binding_info.stride() * description.num_vertices),
    m_binding_info(description.binding_info),
    m_num_vertices(description.num_vertices)
  {
  }
  DEFINE_RESOURCE_DEFINITION(vertex_buffer, vertex_buffer_description, vertex_buffer_state)
}
DEFINE_RESOURCE_TEMPLATES(j::rendering::vulkan::geometry::vertex_buffer, j::rendering::vulkan::geometry::vertex_buffer_description);
