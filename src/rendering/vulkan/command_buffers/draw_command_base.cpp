#include "draw_command_base.hpp"

#include "command_buffer_state.hpp"

namespace j::rendering::vulkan::inline command_buffers {
  draw_command_base::draw_command_base() noexcept {
    this->is_bound_to_epoch(epochs::in_render_pass);
    this->insertion_options = state::prefers_bottom_insert_flag;
    this->starts_epoch(epochs::draw_command);
  }

  draw_command_base::draw_command_base(
    state::node_insertion_context & context,
    const u32_t num,
    const state::condition_initializer * const conditions)
    : draw_command_base()
  {
    for (u32_t i = 0; i < num; ++i) {
      context.add_condition(conditions[i]);
    }
  }
}
