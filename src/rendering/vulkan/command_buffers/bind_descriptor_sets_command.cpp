#include "bind_descriptor_sets_command.hpp"

#include "command_buffer_state.hpp"
#include "command_context.hpp"
#include "rendering/vulkan/descriptors/descriptor_binding_description.hpp"
#include "rendering/vulkan/descriptors/descriptor_binding_state.hpp"
#include "rendering/vulkan/descriptors/descriptor_set.hpp"
#include "rendering/vulkan/descriptors/descriptor_set_layout.hpp"
#include "rendering/vulkan/rendering/pipeline.hpp"
#include "rendering/vulkan/context/render_context.hpp"
#include "strings/format.hpp"

namespace j::rendering::vulkan::inline command_buffers {
  bind_descriptor_sets_command::bind_descriptor_sets_command(
    state::node_insertion_context & context,
    u32_t set_index,
    descriptors::descriptor_set_ref descriptor_set,
    u32_t dynamic_binding_offset)
    : m_descriptor_set(static_cast<descriptors::descriptor_set_ref &&>(descriptor_set)),
      m_set_index(set_index),
      m_dynamic_binding_offset(dynamic_binding_offset)
  {
    this->is_bound_to_current_epoch(command_buffers::epochs::pipeline_binding);
    J_ASSERT(set_index < 16U, "Descriptor set index out of range.");
    J_ASSERT_NOT_NULL(m_descriptor_set);
    context.add_postcondition(descriptors::descriptor_sets[m_set_index]
                      = descriptors::descriptor_set_binding(m_descriptor_set, m_dynamic_binding_offset));
  }

  void bind_descriptor_sets_command::execute(command_context & context, state::reserved_resources_t &) const {
    auto & ds = context.render_context->get(m_descriptor_set);
    const VkDescriptorSet set = ds.vk_descriptor_set;
    const auto & pl = context.render_context->get(context.current_pipeline);
    u32_t num_dynamic = 0U;
    auto & l = context.render_context->get(ds.layout);
    for (auto & b : l.bindings) {
      if (b.binding.type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC) {
        ++num_dynamic;
      }
    }
    J_ASSERT(num_dynamic <= 1, "TODO: Implement multiple dynamics");
    J_ASSERT(num_dynamic == 1 || !m_dynamic_binding_offset, "Dynamic offset given but no dynamic binding.");
    vkCmdBindDescriptorSets(context.vk_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            pl.vk_pipeline_layout,
                            m_set_index,
                            1, &set,
                            num_dynamic, num_dynamic ? &m_dynamic_binding_offset : nullptr);
  }

  bind_descriptor_sets_command::~bind_descriptor_sets_command() {
  }

  strings::string bind_descriptor_sets_command::name() const {
    return strings::format("Bind descriptor set {}", m_set_index);
  }
}
