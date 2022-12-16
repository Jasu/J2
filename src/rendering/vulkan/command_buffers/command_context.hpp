#pragma once

#include <vulkan/vulkan.h>

#include "rendering/vulkan/rendering/pipeline_resource.hpp"

namespace j::rendering::vulkan::rendering {
  struct framebuffer;
}

namespace j::rendering::vulkan::inline context {
  struct render_context;
}

namespace j::rendering::vulkan::inline command_buffers {
  /// Context in which the command will be run.
  ///
  /// Contains references to resources needed during command execution.
  struct command_context {
    J_ALWAYS_INLINE explicit command_context(const render_context & render_context) noexcept
      : render_context(&render_context)
    { }

    const rendering::framebuffer & framebuffer() const noexcept;

    void reset_current_pipeline() noexcept;

    VkCommandBuffer vk_command_buffer = VK_NULL_HANDLE;
    const struct render_context * render_context = nullptr;
    rendering::pipeline_ref current_pipeline;
  };
}
