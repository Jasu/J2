#include "command_context.hpp"
#include "rendering/vulkan/context/render_context.hpp"

namespace j::rendering::vulkan::inline command_buffers {
  const rendering::framebuffer & command_context::framebuffer() const noexcept {
    return *render_context->framebuffer;
  }

  void command_context::reset_current_pipeline() noexcept {
    current_pipeline.reset();
  }
}
