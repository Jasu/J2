#pragma once

#include "command.hpp"
#include "rendering/vulkan/rendering/pipeline_resource.hpp"

namespace j::rendering::vulkan::inline command_buffers {
  class bind_pipeline_command final : public command {
  public:
    bind_pipeline_command(state::node_insertion_context & context, rendering::pipeline_ref && pipeline);
    ~bind_pipeline_command();

    void execute(command_context & context, state::reserved_resources_t &) const override;

    strings::string name() const override;
  private:
    rendering::pipeline_ref m_pipeline;
  };
}
