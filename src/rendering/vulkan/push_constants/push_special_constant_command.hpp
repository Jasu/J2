#pragma once

#include "rendering/vulkan/command_buffers/command.hpp"
#include "rendering/vulkan/push_constants/special_push_constants.hpp"

namespace j::rendering::vulkan {
  inline namespace push_constants {
    class push_special_constant_command final : public command_buffers::command {
    public:
      push_special_constant_command(state::node_insertion_context & context,
                                    u32_t push_constant_index,
                                    special_push_constant constant);

      void execute(command_buffers::command_context & context, state::reserved_resources_t &) const override;

      strings::string name() const override;

    private:
      u32_t m_push_constant_index:24;
      special_push_constant m_type:8;
    };
  }
}
