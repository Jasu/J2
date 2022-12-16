#include "rendering/renderers/text/text_renderer.hpp"

#include "rendering/renderers/text/draw_string_command.hpp"
#include "rendering/renderers/text/instance.hpp"
#include "rendering/renderers/text/text_renderer_string.hpp"
#include "rendering/renderers/text/text_vertex_data_source_handler.hpp"
#include "rendering/vertex_data/static_vertex_data_source.hpp"
#include "rendering/vulkan/command_buffers/command_buffer_builder.hpp"
#include "rendering/vulkan/context/render_context.hpp"
#include "rendering/vulkan/geometry/vertex_buffer.hpp"
#include "rendering/vulkan/geometry/vertex_buffer_description.hpp"
#include "rendering/vulkan/descriptors/descriptor_set_description.hpp"
#include "rendering/vulkan/descriptors/descriptor_set_layout_description.hpp"
#include "rendering/vulkan/rendering/pipeline_description.hpp"
#include "rendering/vulkan/texture_atlases/texture_atlas_description.hpp"
#include "rendering/vulkan/uniform_buffers/renderer_state_uniform_handler.hpp"
#include "rendering/vulkan/uniform_buffers/uniform_buffer.hpp"
#include "rendering/vulkan/uniform_buffers/uniform_buffer_description.hpp"
#include "exceptions/assert_lite.hpp"
#include "files/paths/path.hpp"
#include "fonts/font_set.hpp"
#include "fonts/shaping/shaper.hpp"
#include "fonts/shaping/positioned_glyph.hpp"
#include "services/service_definition.hpp"
#include "strings/unicode/rope/rope_utf8_view.hpp"

namespace j::rendering::renderers::text {
  namespace r = ::j::rendering;
  namespace a = r::vulkan::attributes;
  namespace vd = r::vertex_data;

  text_renderer::text_renderer(mem::shared_ptr<fonts::shaping::shaper> && shaper,
                               mem::shared_ptr<text_vertex_data_source_handler> && handler) noexcept
    : m_atlas(a::width = atlas_size_px_v, a::height = atlas_size_px_v,
              a::format = j::rendering::images::image_format::rgba32,
              a::name = "Glyph atlas"),
      m_instance_buffer(
        a::attribute["texture_rect"] = &instance::texture_rect,
        a::attribute["screen_pos"] = vd::integer(&instance::screen_pos),
        a::num_vertices = 2048U
      ),
      m_uniform_buffer(a::record = v::uniform_buffer_record(v::renderer_state_uniform_layout)),
      m_descriptor_set_layout(
        a::descriptor[0] = a::combined_image_sampler(a::fragment_stage_only),
        a::descriptor[1] = a::uniform(a::layout = v::renderer_state_uniform_layout, a::vertex_stage_only)
      ),
      m_descriptor_set(m_descriptor_set_layout),
      m_pipeline(
        a::name = "Text pipeline",
        a::fragment_shader = a::shader_configuration(
          a::shader = "text_renderer.frag.spv"
        ),
        a::vertex_shader = a::shader_configuration(
          a::shader = "text_renderer.vert.spv",
          a::spec_constant["atlas_size_px"] = (float)atlas_size_px_v
        ),
        a::descriptor_set_layout = m_descriptor_set_layout,
        a::push_constant[0] = a::vertex_shader_push_constant<geometry::vec2f>(),
        a::push_constant[16] = a::fragment_shader_push_constant<colors::rgbaf>(),
        a::vertex_input = a::vertex_input_state{
          a::instance_attribute_binding = a::binding<instance>(
            a::attribute["texture_rect"] = &instance::texture_rect,
            a::attribute["screen_pos"] = vd::integer(&instance::screen_pos)
          ),
        }
      ),
      m_shaper(static_cast<mem::shared_ptr<fonts::shaping::shaper> &&>(shaper)),
      m_handler(static_cast<mem::shared_ptr<text_vertex_data_source_handler> &&>(handler))
  {
    J_ASSERT_NOT_NULL(m_shaper, m_handler);
  }

  mem::shared_ptr<text_renderer_string> text_renderer::allocate_string(
    strings::const_rope_utf8_view str,
    geometry::vec2f position,
    fonts::font_set & fonts,
    colors::rgba8 color)
  {
    J_ASSERT_NOT_NULL(str, fonts);
    return mem::make_shared<text_renderer_string>(
      this,
      position,
      m_shaper->shape(str, fonts, 72),
      color);
  }

  void text_renderer::draw_string(v::command_buffers::command_buffer_builder & builder,
                                  mem::shared_ptr<text_renderer_string> string)
  {
    J_ASSERT_NOT_NULL(string);
    if (J_UNLIKELY(!string->m_allocation)) {
      if (J_UNLIKELY(!m_is_initialized)) {
        builder.context.get(m_atlas);
        m_uniform = builder.context.get(m_uniform_buffer).allocate();
        J_ASSERT_NOT_NULL(m_uniform);
        m_is_initialized = true;
      }
      string->m_allocation = builder.context.get(m_instance_buffer).allocate(string->size() * sizeof(instance));
    } else {
      if (string->m_stamp != m_handler->get_userdata({ m_handler->index(), string.get() })) {
        string->m_allocation.userdata() = 0ULL;
      }
    }
    builder.create_command<draw_string_command>(*this, static_cast<mem::shared_ptr<text_renderer_string> &&>(string));
  }

  text_renderer::~text_renderer() {
  }

  namespace {
    namespace s = services;
    J_A(ND, NODESTROY) const s::service_definition<text_renderer> def(
      "rendering.renderers.text_renderer",
      "Vulkan Text Renderer",
      s::create = s::constructor<
        mem::shared_ptr<fonts::shaping::shaper> &&,
        mem::shared_ptr<text_vertex_data_source_handler> &&
      >()
    );
  }
}
