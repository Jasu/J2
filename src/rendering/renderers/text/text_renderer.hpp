#pragma once

#include "fonts/font_set.hpp"
#include "colors/rgb.hpp"
#include "geometry/vec2.hpp"
#include "mem/shared_ptr_fwd.hpp"
#include "rendering/vulkan/descriptors/descriptor_set_layout_resource.hpp"
#include "rendering/vulkan/descriptors/descriptor_set_resource.hpp"
#include "rendering/vulkan/geometry/vertex_buffer_resource.hpp"
#include "rendering/vulkan/texture_atlases/texture_atlas_resource.hpp"
#include "rendering/vulkan/rendering/pipeline_resource.hpp"
#include "rendering/vulkan/uniform_buffers/uniform_buffer_allocation.hpp"

namespace j::fonts::shaping { class shaper; }
namespace j::rendering::vulkan::inline command_buffers { struct command_buffer_builder; }
namespace j::strings::inline unicode::inline rope {
  template<bool> class basic_rope_utf8_view;
  using const_rope_utf8_view = basic_rope_utf8_view<true>;
}

namespace j::rendering::renderers::text {
  namespace r = ::j::rendering;
  namespace v = r::vulkan;

  class text_vertex_data_source_handler;
  class text_renderer_string;

  class text_renderer final {
  public:
    constexpr static inline u32_t atlas_size_px_v = 512U;
    text_renderer(mem::shared_ptr<fonts::shaping::shaper> && shaper,
                  mem::shared_ptr<text_vertex_data_source_handler> && handler) noexcept;

    mem::shared_ptr<text_renderer_string> allocate_string(
      strings::const_rope_utf8_view str,
      geometry::vec2f position,
      fonts::font_set & fonts,
      colors::rgba8 color);

    void draw_string(v::command_buffers::command_buffer_builder & builder,
                     mem::shared_ptr<text_renderer_string> string);

    ~text_renderer();

  private:
    v::texture_atlas_ref m_atlas;
    v::geometry::vertex_buffer_ref m_instance_buffer;
    v::uniform_buffer_ref m_uniform_buffer;
    v::descriptors::descriptor_set_layout_ref m_descriptor_set_layout;
    v::descriptors::descriptor_set_ref m_descriptor_set;
    v::rendering::pipeline_ref m_pipeline;
    v::uniform_buffer_allocation m_uniform;
    mem::shared_ptr<fonts::shaping::shaper> m_shaper;
    mem::shared_ptr<text_vertex_data_source_handler> m_handler;
    bool m_is_initialized = false;

    friend class draw_string_command;
    friend class text_vertex_data_source_handler;
  };
}
