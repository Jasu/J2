#pragma once

#include "draw_command_base.hpp"
#include "rendering/vulkan/rendering/indirect_buffer_resource.hpp"

namespace j::rendering::vulkan::inline command_buffers {
  class draw_indirect_count_command final : public draw_command_base {
  public:
    template<typename... Conditions, u32_t N>
    J_ALWAYS_INLINE draw_indirect_count_command(state::node_insertion_context & context,
                                                rendering::indirect_count_buffer_ref buffer,
                                                const state::condition_instance (& conditions)[N])
      : draw_command_base(context, N, &conditions[0]),
        m_buffer(static_cast<rendering::indirect_count_buffer_ref &&>(buffer))
    {
      J_ASSERT_NOT_NULL(m_buffer);
    }


    ~draw_indirect_count_command();

    void execute(command_context & context, state::reserved_resources_t &) const override;

    strings::string name() const override;
  private:
    rendering::indirect_count_buffer_ref m_buffer;
  };
}
