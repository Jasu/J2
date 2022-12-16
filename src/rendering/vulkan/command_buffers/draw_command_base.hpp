#pragma once

#include "command.hpp"

namespace j::rendering::vulkan::inline command_buffers {
  class draw_command_base : public command {
  protected:
    draw_command_base() noexcept;
    draw_command_base(state::node_insertion_context & context,
                      u32_t num, const state::condition_initializer * conditions);
  };
}
