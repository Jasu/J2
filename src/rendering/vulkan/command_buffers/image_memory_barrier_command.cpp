#include "image_memory_barrier_command.hpp"
#include "command_context.hpp"
#include "command_buffer_state.hpp"
#include "rendering/vulkan/context/render_context.hpp"
#include "rendering/vulkan/images/image.hpp"
#include "rendering/vulkan/images/image_state.hpp"
#include "rendering/vulkan/synchronization/image_memory_barrier_description.hpp"
#include "rendering/vulkan/state/get_state.hpp"
#include "strings/format.hpp"

namespace j::rendering::vulkan::inline command_buffers {
  image_memory_barrier_command::image_memory_barrier_command(
    state::node_insertion_context & context,
    synchronization::image_memory_barrier_description && barrier
  ) : m_source_access_flags(barrier.wait_for_access_flags()),
      m_destination_access_flags(barrier.postpone_access_flags()),
      m_source_stage_flags(barrier.wait_for_stage_flags()),
      m_destination_stage_flags(barrier.postpone_stage_flags()),
      m_next_layout(barrier.next_layout()),
      m_image(barrier.image())
  {
    J_ASSERT_NOT_NULL(m_image);
    this->is_bound_to_epoch(epochs::before_render_pass);
    context.add_postcondition(images::image_layout(m_image) = m_next_layout);
  }

  void image_memory_barrier_command::execute(command_context & context, state::reserved_resources_t &) const {
    const VkImageMemoryBarrier barrier{
      .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
      .srcAccessMask = m_source_access_flags,
      .dstAccessMask = m_destination_access_flags,
      .oldLayout = state::get_state(m_image).layout,
      .newLayout = m_next_layout,
      .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .image = context.render_context->get(m_image).vk_image,
      .subresourceRange = {
        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
        .baseMipLevel = 0,
        .levelCount = 1,
        .baseArrayLayer = 0,
        .layerCount = 1,
      },
    };
    state::get_state(m_image).layout = m_next_layout;
    vkCmdPipelineBarrier(context.vk_command_buffer,
                         m_source_stage_flags, m_destination_stage_flags,
                         VK_DEPENDENCY_BY_REGION_BIT,
                         0, nullptr,
                         0, nullptr,
                         1, &barrier);
  }

  strings::string image_memory_barrier_command::name() const {
    return "Image-memory barrier";
  }

  strings::string image_memory_barrier_command::debug_details() const {
    return strings::format("To {}", m_next_layout);
  }
}
