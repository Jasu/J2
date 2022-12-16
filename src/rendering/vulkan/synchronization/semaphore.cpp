#include "rendering/vulkan/synchronization/semaphore.hpp"

#include "rendering/vulkan/context/device_context_base.hpp"
#include "rendering/vulkan/device.hpp"
#include "rendering/vulkan/utils.hpp"

namespace {
  const VkSemaphoreCreateInfo semaphore_info{
    .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
  };
}

namespace j::rendering::vulkan::synchronization {
  semaphore::semaphore(const device_context_base & context) {
    J_ASSERT(!vk_semaphore, "Double-init");
    J_VK_CALL_CHECKED(vkCreateSemaphore, context.device().vk_device,
                      &semaphore_info, nullptr, &vk_semaphore);
  }

  void semaphore::finalize(const device_context_base & context) {
    J_ASSERT_NOT_NULL(vk_semaphore);
    vkDestroySemaphore(context.device().vk_device, vk_semaphore, nullptr);
    vk_semaphore = VK_NULL_HANDLE;
  }
}
