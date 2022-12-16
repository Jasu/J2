#include "rendering/vulkan/images/image_state.hpp"

#include "rendering/vulkan/synchronization/image_memory_barrier_description.hpp"
#include "rendering/vulkan/command_buffers/image_memory_barrier_command.hpp"
#include "rendering/vulkan/command_buffers/copy_from_buffer_to_image_command.hpp"
#include "rendering/images/image_source_manager.hpp"
#include "strings/format.hpp"
#include "rendering/images/image_source_manager.hpp"

namespace j::rendering::vulkan::images {
  namespace i = j::rendering::images;
  namespace s = j::rendering::vulkan::state;

  namespace {
    enum class image_state_variable {
      image_layout = 0x100,
      image_contents = 0x101,
    };
  }

  const image_layout_var image_layout{image_state_variable::image_layout};

  bool image_layout_var::can_transition(const s::transition_context &,
                                        VkImageLayout, VkImageLayout to) const
  { return to != VK_IMAGE_LAYOUT_UNDEFINED; }

  s::condition_group * image_layout_var::transition(
    s::transition_context & context,
    image_ref ref,
    [[maybe_unused]] VkImageLayout from,
    VkImageLayout to
  ) const {
    J_ASSERT_NOT_NULL(ref);
    J_ASSERT(from == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL || from == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL || from == VK_IMAGE_LAYOUT_UNDEFINED,
             "TODO: Unsupported `to` layout.");
    J_ASSERT(to == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL || to == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
             "TODO: Unsupported `to` layout.");
    return ::new command_buffers::image_memory_barrier_command(
      context.context,
      synchronization::image_memory_barrier_description(
        static_cast<image_ref &&>(ref),
        synchronization::next_layout = to,
        // TODO track the access state really
        synchronization::postpone = (to == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
          ? synchronization::transfer_write
          : synchronization::fragment_shader_read,
        synchronization::wait_for = (to == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
          ? synchronization::fragment_shader_read
          : synchronization::transfer_write
    ));
  }

  strings::string image_layout_var::describe_precondition(
    resources::resource_wrapper *, uptr_t,
    const s::precondition_instance & c
  ) const {
    return strings::format("Layout must be {}", static_cast<VkImageLayout>(c.data));
  }

  strings::string image_layout_var::describe_postcondition(
    resources::resource_wrapper *, uptr_t,
    const s::postcondition_instance & c
  ) const {
    return strings::format("Transition to {}", static_cast<VkImageLayout>(c.data));
  }

  bool image_contents_var::can_transition(const s::transition_context &,
                                          i::image_source_key from, i::image_source_key to) const
  {
    return to != from && !to.empty();
  }

  s::condition_group * image_contents_var::transition(
    s::transition_context & context,
    image_ref ref,
    [[maybe_unused]] i::image_source_key from,
    i::image_source_key to) const
  {
    J_ASSERT(can_transition(context, from, to), "Cannot transition to image contents.");
    J_ASSERT_NOT_NULL(ref);
    return ::new command_buffers::copy_from_buffer_to_image_command(
      context.context,
      static_cast<image_ref &&>(ref),
      to,
      (*this)(ref) = to
    );
  }

  strings::string image_contents_var::describe_precondition(
    resources::resource_wrapper *, uptr_t,
    const s::precondition_instance & c
  ) const {
    return strings::format("Image must contain {}",
                           i::image_source_manager::instance.describe(i::image_source_key(c.data)));
  }

  strings::string image_contents_var::describe_postcondition(
    resources::resource_wrapper *, uptr_t,
    const s::postcondition_instance & c
  ) const {
    return strings::format("Set image contents to {}",
                           i::image_source_manager::instance.describe(i::image_source_key(c.data)));
  }

  const image_contents_var image_content{image_state_variable::image_contents};
}
