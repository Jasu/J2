#pragma once

#include "rendering/vulkan/buffers/suballocated_buffer.hpp"
#include "rendering/vertex_data/vertex_buffer_info.hpp"
#include "rendering/vulkan/geometry/vertex_buffer_suballocation.hpp"

namespace j::rendering::vulkan::geometry {
  struct vertex_buffer_description;

  class host_visible_vertex_buffer : public buffers::suballocated_buffer<vertex_buffer_ref> {
  public:
    host_visible_vertex_buffer(const device_context_base & context,
                               const vertex_buffer_description & description);

    J_INLINE_GETTER vertex_data::vertex_buffer_info info() const noexcept
    { return vertex_data::vertex_buffer_info(&m_binding_info, m_num_vertices); }
  private:
    vertex_data::vertex_binding_info m_binding_info;
    u32_t m_num_vertices;
  };

  using vertex_buffer = host_visible_vertex_buffer;
}
