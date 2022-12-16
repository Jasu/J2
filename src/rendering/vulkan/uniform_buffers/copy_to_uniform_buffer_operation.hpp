#pragma once

#include "rendering/vulkan/operations/operation.hpp"

#include "rendering/vulkan/uniform_buffers/uniform_data_source_key.hpp"
#include "rendering/vulkan/uniform_buffers/uniform_view.hpp"

namespace j::rendering::vulkan {
  inline namespace uniform_buffers {
    class copy_to_uniform_buffer_operation final : public operations::operation {
    public:
      copy_to_uniform_buffer_operation(state::node_insertion_context & context,
                                       uniform_data_source_key source,
                                       uniform_view target,
                                       const state::postcondition_initializer & postcondition);

      void execute(command_context & context, state::reserved_resources_t &) const override;

      strings::string name() const override;
    private:
      uniform_data_source_key m_source_key;
      mutable uniform_view m_target;
    };
  }
}
