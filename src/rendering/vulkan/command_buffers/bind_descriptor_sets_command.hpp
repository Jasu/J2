#pragma once

#include "command.hpp"
#include "rendering/vulkan/descriptors/descriptor_set_resource.hpp"

namespace j::rendering::vulkan::inline command_buffers {
  class bind_descriptor_sets_command final : public command {
  public:
    bind_descriptor_sets_command(state::node_insertion_context & context,
                                 u32_t set_index,
                                 descriptors::descriptor_set_ref descriptor_set,
                                 u32_t dynamic_binding_offset);

    ~bind_descriptor_sets_command();

    void execute(command_context & context, state::reserved_resources_t &) const override;

    strings::string name() const override;
  private:
    descriptors::descriptor_set_ref m_descriptor_set;
    u32_t m_set_index;
    u32_t m_dynamic_binding_offset = 0U;
  };
}
