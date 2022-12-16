#include "rendering/renderers/text/text_vertex_data_source_handler.hpp"

#include "rendering/renderers/text/instance.hpp"
#include "rendering/renderers/text/text_renderer.hpp"
#include "rendering/renderers/text/text_renderer_string.hpp"
#include "fonts/rasterizing/font_image_source_handler.hpp"
#include "rendering/vertex_data/get_vertex_attribute_format.hpp"
#include "rendering/vertex_data/vertex_buffer_view.hpp"
#include "rendering/vulkan/texture_atlases/texture_atlas.hpp"
#include "rendering/vulkan/context/render_context.hpp"
#include "services/service_definition.hpp"
#include "strings/format.hpp"

namespace j::rendering::renderers::text {
  namespace {
    const vd::vertex_binding_info instance_info({
        { 0, &instance::texture_rect },
        { 1, vd::integer(&instance::screen_pos) },
    });
  }

  void text_vertex_data_source_handler::copy_to(
    const vulkan::render_context & context,
    vd::vertex_buffer_view & to, vd::vertex_data_source_key source) const {
    J_ASSERT_NOT_NULL(to, source);
    const auto s = get_string(source);
    u8_t * p = reinterpret_cast<u8_t *>(to.data());
    const u32_t stride = to.info().stride_bytes();
    J_ASSERT(to.info().size_bytes() >= s->size() * stride, "String does not fit to target.");
    J_ASSERT_NOT_NULL(p, s->renderer());
    auto & atlas = context.get(s->renderer()->m_atlas).atlas;
    u32_t stamp = 0U;
    const int multiplier = (1U << 16) / text_renderer::atlas_size_px_v;
    for (auto & g : *s->m_string) {
      auto & rec = atlas.at((uptr_t)fonts::rasterizing::glyph_key(fonts::rasterizing::font_image_source_handler::instance.index(),
                                                                  s->m_string->font_index(),
                                                                  g.glyph_index));
      stamp = ::j::max(stamp, rec.creation_stamp);
      *reinterpret_cast<instance*>(p) = instance{
        geometry::rect_i16(
          rec.rect.left() * multiplier, rec.rect.top() * multiplier,
          rec.rect.width() * multiplier, rec.rect.height() * multiplier),
        geometry::vec2i16((g.x + rec.bearing.x) * 4U, (g.y - rec.bearing.y) * 4U),
      };
      p += stride;
    }
    s->m_stamp = stamp;
  }

  strings::string text_vertex_data_source_handler::describe(vd::vertex_data_source_key source) const {
    return strings::format("String at 0x{:012X}", reinterpret_cast<uptr_t>(get_string(source)));
  }

  u64_t text_vertex_data_source_handler::get_userdata(vd::vertex_data_source_key source) const {
    const auto s = get_string(source);
    auto & atlas = s->renderer()->m_atlas.get().atlas;
    u64_t ud = 0U;
    for (auto & g : *s->m_string) {
      auto tex = atlas.maybe_at((uptr_t)fonts::rasterizing::glyph_key(fonts::rasterizing::font_image_source_handler::instance.index(),
                                                                  s->m_string->font_index(),
                                                                  g.glyph_index));
      if (!tex) {
        return 0U;
      }
      ud = ::j::max(ud, tex->creation_stamp);
    }
    return ud;
  }

  vd::vertex_buffer_info text_vertex_data_source_handler::get_info(vd::vertex_data_source_key source) const {
    return vd::vertex_buffer_info(&instance_info, get_string(source)->size());
  }

  J_INTERNAL_LINKAGE text_renderer_string * text_vertex_data_source_handler::get_string(vd::vertex_data_source_key source) const {
    J_ASSERT(source.handler_index() == index(), "Wrong vertex source handler.");
    auto ptr = reinterpret_cast<text_renderer_string *>(source.ptr());
    J_ASSERT_NOT_NULL(ptr);
    return ptr;
  }

  namespace {
    namespace s = services;
    J_A(ND, NODESTROY) const s::service_definition<text_vertex_data_source_handler> def(
      "fonts.text_vertex_data_source_handler",
      "Text vertex data source handler",
      s::global_singleton,
      s::implements = s::interface<vd::vertex_data_source_handler>
    );
  }
}
