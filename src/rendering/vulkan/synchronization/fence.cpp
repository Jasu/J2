#include "rendering/vulkan/synchronization/fence.hpp"

#include "rendering/vulkan/context/device_context_base.hpp"
#include "rendering/vulkan/device.hpp"
#include "rendering/vulkan/utils.hpp"
#include <unistd.h>

namespace {
  const VkExportFenceCreateInfo fence_export_create_info{
    .sType = VK_STRUCTURE_TYPE_EXPORT_FENCE_CREATE_INFO,
    .pNext = nullptr,
    .handleTypes = VK_EXTERNAL_FENCE_HANDLE_TYPE_SYNC_FD_BIT,
  };
  const VkFenceCreateInfo fence_create_info{
    .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
    .pNext = &fence_export_create_info,
    .flags = VK_FENCE_CREATE_SIGNALED_BIT,
  };
}

namespace j::rendering::vulkan::synchronization {
  fence::fence(const device_context_base & context) {
    auto & device = context.device();
    VkDevice vk_device = device.vk_device;
    J_VK_CALL_CHECKED(vkCreateFence, vk_device, &fence_create_info, nullptr, &vk_fence);
    m_get_fd_info = {
      .sType = VK_STRUCTURE_TYPE_FENCE_GET_FD_INFO_KHR,
      .pNext = nullptr,
      .fence = vk_fence,
      .handleType = VK_EXTERNAL_FENCE_HANDLE_TYPE_SYNC_FD_BIT,
    };
    try {
      J_VK_CALL_CHECKED(device.vkGetFenceFdKHR, vk_device, &m_get_fd_info, &fd);
      J_VK_ASSERT(fd > 2, "Could not export fence to a file descriptor.");
    } catch(...) {
      vkDestroyFence(vk_device, vk_fence, nullptr);
      throw;
    }
  }

  void fence::finalize(const device_context_base & context) {
    if (fd >= 0) {
      J_ASSERT(fd > 2, "No file descriptor.");
      close(fd);
      fd = -1;
    }
    J_ASSERT_NOT_NULL(vk_fence);
    auto d = context.device().vk_device;
    vkWaitForFences(d, 1, &vk_fence, VK_TRUE, 2000000);
    vkDestroyFence(d, vk_fence, nullptr);
    vk_fence = VK_NULL_HANDLE;
  }

  [[nodiscard]] int fence::update_fd(const device_context_base & context) {
    if (fd >= 0) {
      J_ASSERT(fd > 2, "No file descriptor.");
      close(fd);
      fd = -1;
    }
    J_VK_CALL_CHECKED(context.device().vkGetFenceFdKHR, context.device().vk_device, &m_get_fd_info, &fd);
    J_ASSERT(fd > 2, "No file descriptor.");
    return fd;
  }

  bool fence::wait(const device_context_base & context, u64_t timeout_ns) {
    const VkResult result = vkWaitForFences(context.device().vk_device, 1, &vk_fence, VK_TRUE, timeout_ns);
    if (J_LIKELY(result == VK_SUCCESS)) {
      return true;
    } else if (J_LIKELY(result == VK_TIMEOUT)) {
      return false;
    }
    J_VK_FAIL_FUNCTION(vkWaitForFences, result);
  }

  void fence::reset(const device_context_base & context) {
    J_VK_CALL_CHECKED(vkResetFences, context.device().vk_device, 1, &vk_fence);
  }

  void fence::wait_and_reset(const device_context_base & context, u64_t timeout_ns) {
    auto d = context.device().vk_device;
    const VkResult result = vkGetFenceStatus(d, vk_fence);
    if (J_LIKELY(result == VK_SUCCESS || result == VK_NOT_READY)) {
      if (result == VK_NOT_READY) {
        J_VK_CALL_CHECKED(vkWaitForFences, d, 1, &vk_fence, VK_TRUE, timeout_ns);
      }
    } else {
      J_VK_FAIL_FUNCTION(vkGetFenceStatus, result);
    }
    J_VK_CALL_CHECKED(vkResetFences, d, 1, &vk_fence);
  }

  bool fence::is_signaled(const device_context_base & context) const {
    const VkResult result = vkGetFenceStatus(context.device().vk_device, vk_fence);
    if (J_UNLIKELY(result != VK_SUCCESS && result != VK_NOT_READY)) {
      J_VK_FAIL_FUNCTION(vkGetFenceStatus, result);
    }
    return result == VK_SUCCESS;
  }
}
