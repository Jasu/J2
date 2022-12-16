#include "bind_vertex_buffers_command.hpp"

#include "command_context.hpp"
#include "command_buffer_state.hpp"
#include "rendering/vulkan/geometry/vertex_buffer.hpp"
#include "rendering/vulkan/utils.hpp"
#include "rendering/vulkan/context/render_context.hpp"
#include "rendering/vulkan/geometry/vertex_binding_state.hpp"
#include "strings/string.hpp"
#include "containers/vector.hpp"

J_DEFINE_EXTERN_NONCOPYABLE_VECTOR(j::rendering::vulkan::geometry::vertex_buffer_ref);
J_DEFINE_EXTERN_NONCOPYABLE_VECTOR(VkDeviceSize);

namespace j::rendering::vulkan::inline command_buffers {
  bind_vertex_buffers_command::bind_vertex_buffers_command(
    state::node_insertion_context & context,
    u32_t first_binding,
    span<geometry::vertex_buffer_ref> vertex_buffers,
    span<VkDeviceSize> offsets)
    : m_vertex_buffers(containers::copy, vertex_buffers.begin(), vertex_buffers.size()),
      m_offsets(containers::copy, offsets.begin(), offsets.size()),
      m_first_binding(first_binding)
  {
    this->insertion_options = state::merges_with_siblings_flag | state::merges_with_parent_flag;
    J_VK_ASSERT(m_vertex_buffers.size(),
                "Tried to bind zero vertex buffers.");
    J_VK_ASSERT(m_vertex_buffers.size() == m_offsets.size(),
                "Offsets size does not match vertex buffers size.");
    this->is_bound_to_current_epoch(epochs::pipeline_binding);
    for (u8_t i = 0; i < m_vertex_buffers.size(); ++i) {
      context.add_postcondition(geometry::vertex_bindings[first_binding + i] = m_vertex_buffers[i]);
    }
  }

  bind_vertex_buffers_command::bind_vertex_buffers_command(
    state::node_insertion_context & context,
    u32_t binding,
    geometry::vertex_buffer_ref vertex_buffer,
    VkDeviceSize offset)
    : m_vertex_buffers(containers::copy, &vertex_buffer, 1U),
      m_offsets(containers::copy, &offset, 1U),
      m_first_binding(binding)
  {
    context.add_postcondition(geometry::vertex_bindings[binding] = m_vertex_buffers[0]);
    this->is_bound_to_current_epoch(epochs::pipeline_binding);
  }

  void bind_vertex_buffers_command::execute(command_context & context, state::reserved_resources_t &) const {
    const u32_t sz = m_vertex_buffers.size();
    VkBuffer buffers[sz];
    J_ASSERT(sz == m_offsets.size());
    for (sz_t i = 0; i < sz; ++i) {
      buffers[i] = context.render_context->get(m_vertex_buffers[i]).vk_buffer;
    }
    vkCmdBindVertexBuffers(context.vk_command_buffer, m_first_binding, sz, buffers, m_offsets.begin());
  }

  bool bind_vertex_buffers_command::can_merge_with(const state::node_insertion_context &,
                                                   [[maybe_unused]] const condition_group & group) const noexcept {
    J_ASSERT(&group != this, "Cannot merge with self.");
    return true;
  }

  void bind_vertex_buffers_command::merge(state::node_insertion_context & context,
                                          condition_group && group) {
    auto & g = static_cast<bind_vertex_buffers_command &>(group);
    J_ASSERT(&g != this, "Cannot merge with self.");
    if (m_first_binding > g.m_first_binding) {
      m_vertex_buffers.swap(g.m_vertex_buffers);
      m_offsets.swap(g.m_offsets);
    }
    const u8_t sz = m_vertex_buffers.size() + g.m_vertex_buffers.size();
    m_vertex_buffers.reserve(sz);
    m_offsets.resize(sz);
    for (u32_t i = 0; i < size(); ++i) {
      context.move_condition(*this, i);
    }
    for (u32_t i = 0; i < g.m_vertex_buffers.size(); ++i) {
      m_vertex_buffers.push_back(static_cast<geometry::vertex_buffer_ref &&>(g.m_vertex_buffers[i]));
      m_offsets.emplace_back(g.m_offsets[i]);
      context.move_condition(group, i);
    }
  }

  strings::string bind_vertex_buffers_command::name() const {
    return "Bind vertex buffers";
  }
}
