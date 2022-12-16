#pragma once

#include <vulkan/vulkan.h>

#include "command.hpp"
#include "rendering/vulkan/images/image_resource.hpp"

namespace j::rendering::vulkan::synchronization {
  class image_memory_barrier_description;
}

namespace j::rendering::vulkan::inline command_buffers {
  class image_memory_barrier_command final : public command {
    VkAccessFlags m_source_access_flags;
    VkAccessFlags m_destination_access_flags;
    VkPipelineStageFlags m_source_stage_flags;
    VkPipelineStageFlags m_destination_stage_flags;
    VkImageLayout m_next_layout;
    images::image_ref m_image;
  public:
    image_memory_barrier_command(state::node_insertion_context & context,
                                 synchronization::image_memory_barrier_description && barrier);

    void execute(command_context & context, state::reserved_resources_t &) const override;

    strings::string name() const override;

    strings::string debug_details() const override;
  };
}
