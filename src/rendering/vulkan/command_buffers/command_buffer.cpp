#include "command_buffer.hpp"

#include "command.hpp"
#include "command_context.hpp"
#include "rendering/vulkan/context/device_context_base.hpp"
#include "rendering/vulkan/device.hpp"
#include "rendering/vulkan/utils.hpp"

namespace {
  const VkCommandBufferBeginInfo command_buffer_begin_info{
    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
  };
}

namespace j::rendering::vulkan::inline command_buffers {
  command_buffer::command_buffer(const device_context_base & context)
    : device(context.device_ref())
  {
    auto & dev = context.device();
    const VkCommandBufferAllocateInfo buffer_info{
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
      .pNext = nullptr,
      .commandPool = dev.vk_command_pool,
      .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
      .commandBufferCount = 1,
    };
    J_VK_CALL_CHECKED(vkAllocateCommandBuffers, dev.vk_device, &buffer_info, &vk_command_buffer);
  }

  void command_buffer::begin() {
    J_VK_CALL_CHECKED(vkBeginCommandBuffer, vk_command_buffer, &command_buffer_begin_info);
  }

  void command_buffer::end() {
    J_VK_CALL_CHECKED(vkEndCommandBuffer, vk_command_buffer);
  }

  void command_buffer::reset() {
    J_VK_CALL_CHECKED(vkResetCommandBuffer, vk_command_buffer, 0);
  }

  command_buffer::~command_buffer() {
    if (auto dev = device.lock()) {
      vkFreeCommandBuffers(dev.get().vk_device, dev.get().vk_command_pool,
                          1, &vk_command_buffer);
    }
  }
}
