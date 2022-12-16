#pragma once

#include "draw_command_base.hpp"
#include "exceptions/assert_lite.hpp"

namespace j::rendering::vulkan::inline command_buffers {
  class draw_command final : public draw_command_base {
  public:
    draw_command(
      state::node_insertion_context &,
      u16_t vertex_count, u16_t instance_count, u16_t first_vertex, u16_t first_instance)
      : m_vertex_count(vertex_count),
        m_instance_count(instance_count),
        m_first_vertex(first_vertex),
        m_first_instance(first_instance)
    {
      J_ASSERT_NOT_NULL(m_vertex_count, m_instance_count);
    }

    void execute(command_context & context, state::reserved_resources_t &) const override;

    strings::string name() const override;

  private:
    const u16_t m_vertex_count, m_instance_count, m_first_vertex, m_first_instance;
  };
}
