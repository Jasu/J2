#include "bind_pipeline_command.hpp"

#include "command_context.hpp"
#include "rendering/vulkan/rendering/pipeline.hpp"
#include "rendering/vulkan/context/render_context.hpp"
#include "command_buffer_state.hpp"
#include "rendering/vulkan/descriptors/descriptor_binding_state.hpp"
#include "rendering/vulkan/push_constants/push_constant_state.hpp"
#include "strings/format.hpp"

namespace j::rendering::vulkan::inline command_buffers {
  bind_pipeline_command::bind_pipeline_command(
    state::node_insertion_context & context, rendering::pipeline_ref && pipeline)
    : m_pipeline(static_cast<rendering::pipeline_ref &&>(pipeline))
  {
    this->insertion_options = state::prefers_bottom_insert_flag;
    this->is_bound_to_epoch(epochs::in_render_pass);
    this->starts_epoch(epochs::pipeline_binding);
    auto & p = context.render_context.get(m_pipeline);
    const u8_t num_layouts = max(p.descriptor_set_layouts().size(), 1);
    context.add_postcondition(bound_pipeline = m_pipeline);
    for (u8_t i = 0; i < num_layouts; ++i) {
      context.add_postcondition(descriptors::descriptor_sets[i].clear());
    }
    for (auto & c : p.push_constants) {
      context.add_postcondition(push_constant[c.offset].clear());
    }
  }

  bind_pipeline_command::~bind_pipeline_command() {
  }

  void bind_pipeline_command::execute(command_context & context, state::reserved_resources_t &) const {
    context.current_pipeline = m_pipeline;
    vkCmdBindPipeline(context.vk_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                      context.render_context->get(m_pipeline).vk_pipeline);
  }

  strings::string bind_pipeline_command::name() const {
    return (m_pipeline.is_initialized() && m_pipeline.get().name) ? strings::format("Bind pipeline {}", m_pipeline.get().name) : "Bind pipeline";
  }
}
