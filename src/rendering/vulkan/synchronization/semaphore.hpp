#pragma once

#include <vulkan/vulkan.h>

#include "hzd/utility.hpp"

namespace j::rendering::vulkan::inline context {
  class device_context_base;
}

namespace j::rendering::vulkan::synchronization {
  struct semaphore final {
    explicit semaphore(const device_context_base & context);

    void finalize(const device_context_base & context);

    VkSemaphore vk_semaphore = VK_NULL_HANDLE;
  };
}
