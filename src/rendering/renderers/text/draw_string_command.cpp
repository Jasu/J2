#include "rendering/renderers/text/draw_string_command.hpp"

#include "strings/string.hpp"
#include "containers/obstack.hpp"
#include "fonts/rasterizing/font_image_source_handler.hpp"
#include "rendering/vulkan/command_buffers/command_buffer_state.hpp"
#include "rendering/vulkan/command_buffers/command_context.hpp"
#include "rendering/vulkan/descriptors/descriptor_binding_state.hpp"
#include "rendering/vulkan/descriptors/descriptor_set_state.hpp"
#include "rendering/vulkan/geometry/vertex_binding_state.hpp"
#include "rendering/vulkan/geometry/vertex_buffer_state.hpp"
#include "rendering/vulkan/geometry/vertex_buffer_suballocation_state.hpp"
#include "rendering/vulkan/images/image_state.hpp"
#include "rendering/vulkan/rendering/pipeline.hpp"
#include "rendering/vulkan/rendering/reserved_resource.hpp"
#include "rendering/vulkan/context/render_context.hpp"
#include "rendering/vulkan/texture_atlases/texture_atlas.hpp"
#include "rendering/vulkan/texture_atlases/texture_atlas_state.hpp"
#include "rendering/vulkan/uniform_buffers/renderer_state_uniform_handler.hpp"
#include "rendering/vulkan/uniform_buffers/uniform_buffer_state.hpp"
#include "rendering/renderers/text/instance.hpp"
#include "rendering/renderers/text/text_renderer.hpp"
#include "rendering/renderers/text/text_renderer_string.hpp"
#include "rendering/renderers/text/text_vertex_data_source_handler.hpp"

namespace j::rendering::renderers::text {
  namespace {
    struct J_TYPE_HIDDEN draw_string_push {
      geometry::vec2f position;
      alignas(16) colors::rgbaf color;
    };
  }
  draw_string_command::draw_string_command(
    vulkan::state::node_insertion_context & context,
    text_renderer & renderer, mem::shared_ptr<text_renderer_string> string)
    : m_renderer(renderer),
      m_string(static_cast<mem::shared_ptr<text_renderer_string> &&>(string))
  {
    context.add_precondition(v::command_buffers::bound_pipeline == m_renderer.m_pipeline);
    context.add_precondition(v::descriptors::descriptor_sets[0] == m_renderer.m_descriptor_set);
    context.add_precondition(v::descriptors::descriptor_binding(m_renderer.m_descriptor_set, 0) == m_renderer.m_atlas.get().image);
    context.add_precondition(v::descriptors::descriptor_binding(m_renderer.m_descriptor_set, 1) == m_renderer.m_uniform);
    context.add_precondition(v::uniform_contents(m_renderer.m_uniform) == v::renderer_state_uniform_key);
    context.add_precondition(v::geometry::vertex_suballocation_contents(m_string->m_allocation)
                     == j::rendering::vertex_data::vertex_data_source_key(m_renderer.m_handler->index(), m_string.get()));
    context.add_precondition(v::geometry::vertex_bindings[0] == m_renderer.m_instance_buffer);
    for (auto glyph : m_string->string().unique_glyphs()) {
      context.add_precondition(v::texture_atlas_contents.requires_image(
                                 m_renderer.m_atlas,
                                 fonts::rasterizing::glyph_key(
                                   fonts::rasterizing::font_image_source_handler::instance.index(),
                                   m_string->string().font_index(),
                                   glyph)));
    }
    context.add_precondition(v::images::image_layout(m_renderer.m_atlas.get().image) == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
  }

  void draw_string_command::execute(v::command_buffers::command_context & context,
                                    v::state::reserved_resources_t & res) const {
    const auto & pipeline = context.render_context->get(context.current_pipeline);
    colors::rgbaf color = colors::rgba_to_float(m_string->m_color);
    vkCmdPushConstants(context.vk_command_buffer,
                       pipeline.vk_pipeline_layout,
                       VK_SHADER_STAGE_VERTEX_BIT,
                       0U,
                       sizeof(geometry::vec2f),
                       &m_string->m_position);
    vkCmdPushConstants(context.vk_command_buffer,
                       pipeline.vk_pipeline_layout,
                       VK_SHADER_STAGE_FRAGMENT_BIT,
                       16U,
                       sizeof(color),
                       &color);
    vkCmdDraw(
      context.vk_command_buffer,
      4,
      m_string->size(),
      0,
      m_string->m_allocation.offset() / sizeof(instance));
    res.push(v::rendering::reserved_resource(m_string));
  }

  strings::string draw_string_command::name() const {
    return "Draw string";
  }
}
