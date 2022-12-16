#include "fonts/rasterizing/font_image_source_handler.hpp"
#include "fonts/rasterizing/font_rasterizer.hpp"
#include "fonts/rasterizing/glyph_key.hpp"
#include "fonts/rasterizing/glyph_info.hpp"
#include "fonts/rasterizing/rasterizer_font.hpp"
#include "exceptions/assert_lite.hpp"
#include "services/service_definition.hpp"
#include "strings/format.hpp"

namespace j::fonts::rasterizing {
  namespace i = rendering::images;

  void font_image_source_handler::initialize(mem::shared_ptr<font_rasterizer> rasterizer) {
    m_rasterizer = static_cast<mem::shared_ptr<font_rasterizer> &&>(rasterizer);
    J_ASSERT_NOT_NULL(m_rasterizer);
  }

  void font_image_source_handler::copy_to(const r::vulkan::render_context &,
                                          i::image_buffer_view & to, i::image_source_key source_key) const {
    J_ASSERT(source_key.handler_index() == index(), "Invalid handler index.");
    J_ASSERT_NOT_NULL(m_rasterizer);
    m_rasterizer->rasterize_glyph(glyph_key(source_key), to);
  }

  i::image_buffer_info font_image_source_handler::get_info(i::image_source_key source) const {
    J_ASSERT(source.handler_index() == index(), "Invalid handler index.");
    J_ASSERT_NOT_NULL(m_rasterizer);
    const glyph_key k{source};
    auto info = m_rasterizer->get_glyph_info(k);
    return i::image_buffer_info{
      info.width(),
      info.height(),
      i::image_format::bgra32, //info.is_color() ? i::image_format::bgra32 : i::image_format::gray8,
      (u16_t)info.x_bearing().floor(),
      (u16_t)info.y_bearing().floor()
    };
  }

  font_image_source_handler font_image_source_handler::instance;

  strings::string font_image_source_handler::describe(i::image_source_key source) const {
    J_ASSERT(source.handler_index() == index(), "Invalid handler index.");
    const glyph_key k{source};
    auto f = m_rasterizer->get_rasterizer_font(k.font_index());
    return strings::format("\"{}\" in {}{}{}",
                           m_rasterizer->get_glyph_name(k),
                           f ? f->family_name() : "",
                           (f && f->style_name()) ? " " : "",
                           f ? f->style_name() : "");
  }

  namespace {
    namespace s = services;
    J_A(ND, NODESTROY) const s::service_definition<font_image_source_handler> def(
      "fonts.font_image_source_handler",
      "Font image source handler",
      s::global_singleton,
      s::implements = s::interface<i::image_source_handler>,
      s::initialize = s::initializer_call(&font_image_source_handler::initialize),
      s::create = &font_image_source_handler::instance
    );
  }
}
