#include "rendering/renderers/box/box_renderer.hpp"

#include "rendering/renderers/box/box.hpp"
#include "rendering/renderers/box/box_instance_data_source_handler.hpp"
#include "strings/format.hpp"
#include "rendering/vulkan/command_buffers/command_buffer_builder.hpp"
#include "rendering/vulkan/command_buffers/command_buffer_state.hpp"
#include "rendering/vulkan/command_buffers/draw_command.hpp"
#include "rendering/vulkan/descriptors/descriptor_binding_state.hpp"
#include "rendering/vulkan/descriptors/descriptor_set_state.hpp"
#include "rendering/vulkan/descriptors/descriptor_set_description.hpp"
#include "rendering/vulkan/descriptors/descriptor_set_layout_description.hpp"
#include "rendering/vulkan/geometry/vertex_binding_state.hpp"
#include "rendering/vulkan/geometry/vertex_buffer.hpp"
#include "rendering/vulkan/geometry/vertex_buffer_description.hpp"
#include "rendering/vulkan/geometry/vertex_buffer_suballocation_state.hpp"
#include "rendering/vulkan/rendering/pipeline_description.hpp"
#include "rendering/vulkan/context/render_context.hpp"
#include "rendering/vulkan/uniform_buffers/renderer_state_uniform_handler.hpp"
#include "rendering/vulkan/uniform_buffers/uniform_buffer.hpp"
#include "rendering/vulkan/uniform_buffers/uniform_buffer_description.hpp"
#include "rendering/vulkan/uniform_buffers/uniform_buffer_state.hpp"
#include "files/paths/path.hpp"
#include "services/service_definition.hpp"

namespace j::rendering::renderers::box {
  namespace a = r::vulkan::attributes;

  box_renderer::box_renderer(mem::shared_ptr<box_instance_data_source_handler> instance_handler) noexcept
    : m_instance_buffer(
        a::attribute[0] = &instance::rectangle_px,
        a::attribute[1] = &instance::background_color,
        a::attribute[2] = &instance::border_left_color,
        a::attribute[3] = &instance::border_right_color,
        a::attribute[4] = &instance::border_top_color,
        a::attribute[5] = &instance::border_bottom_color,
        a::attribute[6] = &instance::border_size,
        a::attribute[7] = &instance::corner_radius,
        a::num_vertices = 512
      ),
      m_uniform_buffer(a::record = v::uniform_buffer_record(
                         v::renderer_state_uniform_layout)),
      m_descriptor_set_layout(
        a::descriptor[0] = a::uniform(
          a::layout = v::renderer_state_uniform_layout,
          a::vertex_stage_only)
      ),
      m_descriptor_set(m_descriptor_set_layout),
      m_pipeline(
        a::name = "Box pipeline",
        a::fragment_shader = "box_renderer.frag.spv",
        a::vertex_shader = "box_renderer.vert.spv",
        a::descriptor_set_layout = m_descriptor_set_layout,
        a::vertex_input = a::vertex_input_state{
          a::instance_attribute_binding = a::binding<instance>(
            a::attribute["rect"]               = vd::integer(&instance::rectangle_px),
            a::attribute["rgba"]               = &instance::background_color,
            a::attribute["border_left_rgba"]   = &instance::border_left_color,
            a::attribute["border_right_rgba"]  = &instance::border_right_color,
            a::attribute["border_top_rgba"]    = &instance::border_top_color,
            a::attribute["border_bottom_rgba"] = &instance::border_bottom_color,
            a::attribute["border_size"]        = vd::integer(&instance::border_size),
            a::attribute["corner_radius"]      = vd::integer(&instance::corner_radius)
          ),
        }
     ),
      m_instance_handler(static_cast<mem::shared_ptr<box_instance_data_source_handler> &&>(instance_handler))
  {
    J_ASSERT_NOT_NULL(m_instance_handler);
  }

  void box_renderer::draw_box(v::command_buffers::command_buffer_builder & builder, const box & box) {
    builder.context.get(m_pipeline);
    if (J_UNLIKELY(!box.m_allocation)) {
      box.m_allocation = builder.context.get(m_instance_buffer).allocate(sizeof(instance));
      if (J_UNLIKELY(!m_uniform)) {
        m_uniform = builder.context.get(m_uniform_buffer).allocate();
      }
    }
    v::state::node_insertion_context c{builder.insertion_context()};
    c.begin_add_node();
    auto cmd = ::new v::command_buffers::draw_command(
      c,
      4U, 1U, 0U,
      box.m_allocation.offset() / sizeof(instance));
    c.add_precondition(v::command_buffers::bound_pipeline == m_pipeline);
    c.add_precondition(v::descriptors::descriptor_sets[0] == m_descriptor_set);
    c.add_precondition(v::descriptors::descriptor_binding(m_descriptor_set, 0) == m_uniform);
    c.add_precondition(v::uniform_contents(m_uniform) == v::renderer_state_uniform_key);
    c.add_precondition(v::geometry::vertex_suballocation_contents(box.m_allocation)
                       == vd::vertex_data_source_key(m_instance_handler->index(), &box));
    c.add_precondition(v::geometry::vertex_bindings[0] == m_instance_buffer);
    c.finish_add_node(cmd);
    builder.add_command(cmd);
  }
  namespace {
    namespace s = services;
    J_A(ND, NODESTROY) const s::service_definition<box_renderer> def(
      "rendering.renderers.box_renderer",
      "Vulkan Box Renderer",
      s::create = s::constructor<mem::shared_ptr<box_instance_data_source_handler>>()
    );
  }
}
