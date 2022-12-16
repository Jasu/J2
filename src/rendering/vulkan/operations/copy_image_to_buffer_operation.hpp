#pragma once

#include "rendering/vulkan/operations/operation.hpp"

#include "rendering/images/image_source_key.hpp"
#include "rendering/images/image_buffer_view.hpp"

namespace j::rendering::vulkan::operations {
  class copy_image_to_buffer_operation final : public operation {
  public:
    copy_image_to_buffer_operation(
      state::node_insertion_context & context,
      images::image_source_key image,
      images::image_buffer_view target,
      const state::postcondition_initializer & postcondition
    );

    void execute(command_context & context, state::reserved_resources_t &) const override;

    strings::string name() const override;

  private:
    images::image_source_key m_image_source_key;
    mutable images::image_buffer_view m_target;
  };
}
