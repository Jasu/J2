#pragma once

#include "rendering/vulkan/operations/operation.hpp"

#include "rendering/vertex_data/vertex_data_source_key.hpp"
#include "rendering/vertex_data/vertex_buffer_view.hpp"

namespace j::rendering::vulkan::operations {
  class copy_to_vertex_buffer_operation : public operation {
  public:
    copy_to_vertex_buffer_operation(
      state::node_insertion_context & context,
      vertex_data::vertex_data_source_key source,
      vertex_data::vertex_buffer_view target,
      const state::postcondition_initializer & postcondition
    );

    void execute(command_context & context, state::reserved_resources_t &) const override;

    strings::string name() const override;
  private:
    vertex_data::vertex_data_source_key m_source_key;
    mutable vertex_data::vertex_buffer_view m_target;
  };
}
