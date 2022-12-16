#pragma once

#include "command.hpp"

namespace j::rendering::vulkan::inline command_buffers {
  class end_render_pass_command final : public command {
  public:
    end_render_pass_command(state::node_insertion_context & context);
    void execute(command_context & context, state::reserved_resources_t &) const override;
    strings::string name() const override;
  };
}
