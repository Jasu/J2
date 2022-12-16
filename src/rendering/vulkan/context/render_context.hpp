#pragma once

#include "rendering/vulkan/context/surface_context.hpp"

namespace j::rendering::vulkan::rendering {
  struct framebuffer;
}

namespace j::rendering::vulkan::inline context {
  struct render_context final : public surface_context {
    render_context(const mem::shared_ptr<const device_context_base> & device_context,
                   surfaces::surface & surface,
                   rendering::framebuffer & framebuffer) noexcept
      : surface_context(device_context, surface),
        framebuffer(&framebuffer)
    { }

    render_context(const render_context &) = delete;
    render_context & operator=(const render_context &) = delete;

    rendering::framebuffer * framebuffer = nullptr;
  };
}
