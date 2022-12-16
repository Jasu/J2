#pragma once

#include "rendering/vulkan/geometry/vertex_buffer_suballocation.hpp"
#include "fonts/shaping/shaped_string.hpp"
#include "colors/rgb.hpp"
#include "geometry/vec2.hpp"

namespace j::rendering::renderers::text {
  namespace r = ::j::rendering;
  namespace v = r::vulkan;
  class text_renderer;
  class text_renderer_string final {
  public:
    text_renderer_string(text_renderer * renderer,
                         geometry::vec2f pos,
                         mem::shared_ptr<fonts::shaping::shaped_string> string,
                         colors::rgba8 color);

    text_renderer_string(text_renderer_string &&) = delete;
    text_renderer_string & operator=(text_renderer_string &&) = delete;

    J_INLINE_GETTER explicit operator bool() const noexcept {
      return !m_string->empty();
    }

    J_INLINE_GETTER bool operator!() const noexcept {
      return m_string->empty();
    }

    J_INLINE_GETTER u32_t size() const noexcept {
      return m_string->size();
    }

    J_INLINE_GETTER text_renderer * renderer() const noexcept {
      return m_renderer;
    }

    J_INLINE_GETTER const geometry::vec2f & position() const noexcept {
      return m_position;
    }

    J_INLINE_GETTER const fonts::shaping::shaped_string & string() const noexcept {
      return *m_string;
    }

    ~text_renderer_string();

    text_renderer_string(const text_renderer_string &) = delete;
    text_renderer_string & operator=(const text_renderer_string &) = delete;
  private:
    text_renderer * const m_renderer = nullptr;
    v::geometry::vertex_buffer_suballocation m_allocation;
    colors::rgba8 m_color;
    mutable u32_t m_stamp = 0U;
    geometry::vec2f m_position;
    mem::shared_ptr<fonts::shaping::shaped_string> m_string;
    friend class text_renderer;
    friend class draw_string_command;
    friend class text_vertex_data_source_handler;
  };
}
