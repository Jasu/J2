#pragma once

#include <vulkan/vulkan.h>

#include "command.hpp"
#include "containers/span.hpp"
#include "containers/vector.hpp"
#include "rendering/vulkan/geometry/vertex_buffer_resource.hpp"

J_DECLARE_EXTERN_NONCOPYABLE_VECTOR(j::rendering::vulkan::geometry::vertex_buffer_ref);
J_DECLARE_EXTERN_NONCOPYABLE_VECTOR(VkDeviceSize);

namespace j::rendering::vulkan::inline command_buffers {
  class bind_vertex_buffers_command final : public command {
  public:
    bind_vertex_buffers_command(state::node_insertion_context & context,
                                u32_t binding,
                                geometry::vertex_buffer_ref vertex_buffer,
                                VkDeviceSize offset);
    bind_vertex_buffers_command(state::node_insertion_context & context,
                                u32_t first_binding,
                                span<geometry::vertex_buffer_ref> vertex_buffers,
                                span<VkDeviceSize> offsets);

    void execute(command_context & context, state::reserved_resources_t &) const override;

    bool can_merge_with(const state::node_insertion_context & context,
                        const condition_group & group) const noexcept override;

    void merge(state::node_insertion_context & context, condition_group && group) override;

    strings::string name() const override;
  private:
    noncopyable_vector<geometry::vertex_buffer_ref> m_vertex_buffers;
    noncopyable_vector<VkDeviceSize> m_offsets;
    u32_t m_first_binding;
  };
}
