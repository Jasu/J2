#pragma once

#include "geometry/rect.hpp"
#include "mem/shared_ptr_fwd.hpp"

namespace j::rendering::renderers::box { class box_renderer; }
namespace j::rendering::renderers::text { class text_renderer; }
namespace j::rendering::vulkan::inline command_buffers { struct command_buffer_builder; }

namespace j::ui {
  class element;
}
namespace j::ui::inline text {
  class paragraph_layout_engine;
}
namespace j::ui::inline rendering {
  namespace v = j::rendering::vulkan;
  namespace r = j::rendering::renderers;
  namespace g = j::geometry;
  class renderer {
  public:
    renderer(mem::shared_ptr<r::box::box_renderer> && box_renderer,
             mem::shared_ptr<r::text::text_renderer> && text_renderer,
             mem::shared_ptr<paragraph_layout_engine> && ple);

    /// Render element, covering region.
    void render(v::command_buffer_builder & builder,
                const element & element,
                const g::rect_u16 & region);

    /// Render element, covering the whole surface.
    void render(v::command_buffer_builder & builder,
                const element & element);
  private:
    mem::shared_ptr<r::box::box_renderer> m_box_renderer;
    mem::shared_ptr<r::text::text_renderer> m_text_renderer;
    mem::shared_ptr<paragraph_layout_engine> m_paragraph_layout_engine;
  };
}
