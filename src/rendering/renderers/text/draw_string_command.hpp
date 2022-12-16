#pragma once

#include "rendering/vulkan/command_buffers/draw_command_base.hpp"
#include "mem/shared_ptr_fwd.hpp"

namespace j::rendering::renderers::text {
  namespace v = j::rendering::vulkan;

  class text_renderer_string;
  class text_renderer;

  class draw_string_command final : public v::command_buffers::draw_command_base {
  public:
    draw_string_command(
      vulkan::state::node_insertion_context & context,
      text_renderer & renderer, mem::shared_ptr<text_renderer_string> string);

    void execute(v::command_buffers::command_context & context,
                 v::state::reserved_resources_t & res) const override;

    strings::string name() const override;
  private:
    text_renderer & m_renderer;
    mem::shared_ptr<text_renderer_string> m_string;
  };
}
