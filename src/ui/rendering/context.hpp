#pragma once

#include "mem/shared_ptr_fwd.hpp"

namespace j::rendering::renderers::box { class box_renderer; }
namespace j::rendering::renderers::text { class text_renderer; }
namespace j::rendering::vulkan::inline command_buffers { struct command_buffer_builder; }

namespace j::ui::inline text {
  class paragraph_layout_engine;
}
namespace j::ui::inline rendering {
  namespace v = j::rendering::vulkan;
  namespace r = j::rendering::renderers;
  struct context final {
    v::command_buffer_builder * builder;
    r::box::box_renderer      * box_renderer;
    r::text::text_renderer    * text_renderer;
    paragraph_layout_engine   * paragraph_layout_engine;
  };
}
