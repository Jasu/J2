#pragma once

#include <vulkan/vulkan.h>
#include "rendering/vulkan/device_resource.hpp"

namespace j::rendering::vulkan::inline command_buffers {
  struct command_buffer final {
    /// Construct an empty command buffer.
    command_buffer(const device_context_base & context);

    void begin();

    void end();

    /// Clear the command buffer.
    void reset();

    ~command_buffer();

    command_buffer(const command_buffer &) = delete;
    command_buffer & operator=(const command_buffer &) = delete;

    VkCommandBuffer vk_command_buffer = VK_NULL_HANDLE;
    weak_device_ref device;
  };
}
