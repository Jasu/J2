#pragma once

#include <vulkan/vulkan.h>
#include "hzd/utility.hpp"

namespace j::windowing::x11 {
  class context;
}

namespace j::rendering::vulkan::inline context {
  class device_context_base;
}

namespace j::rendering::vulkan {
  struct device final {
    /// Array of required device extension names. Terminated by a nullptr.
    static const char * const required_device_extensions[];

    explicit device(const device_context_base & context);

    ~device();

    void wait_device_idle();

    VkDevice vk_device = VK_NULL_HANDLE;
    VkQueue vk_graphics_queue = VK_NULL_HANDLE;
    VkQueue vk_present_queue = VK_NULL_HANDLE;
    VkCommandPool vk_command_pool = VK_NULL_HANDLE;
    u32_t graphics_queue_index = 0;
    u32_t present_queue_index = 0;
  public:
    PFN_vkGetFenceFdKHR vkGetFenceFdKHR;
    PFN_vkCmdDrawIndirectCountKHR vkCmdDrawIndirectCountKHR;
  };
}
