#include "rendering/vulkan/geometry/vertex_buffer_resource.hpp"

#include "rendering/vulkan/resources/resource_definition.hpp"
#include "rendering/vulkan/geometry/vertex_buffer_description.hpp"
#include "rendering/vulkan/geometry/vertex_buffer_state.hpp"
#include "rendering/vulkan/geometry/vertex_buffer.hpp"

namespace j::rendering::vulkan::geometry {
  DEFINE_RESOURCE_DEFINITION(vertex_buffer, vertex_buffer_description, vertex_buffer_state)
}
DEFINE_RESOURCE_TEMPLATES(j::rendering::vulkan::geometry::vertex_buffer, j::rendering::vulkan::geometry::vertex_buffer_description);
