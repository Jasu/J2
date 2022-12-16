#pragma once

#include <vulkan/vulkan.h>

#include "rendering/vulkan/rendering/render_pass_resource.hpp"
#include "rendering/vulkan/device_resource.hpp"
#include "rendering/vulkan/images/image_view_resource.hpp"

namespace j::rendering::vulkan::rendering {
  /// Specifies which resources are bound to a pipeline.
  struct framebuffer {
    framebuffer(const surface_context & context,
                VkExtent2D vk_extent,
                render_pass_ref render_pass,
                images::image_view_ref image_view);

    ~framebuffer();

    /// Reinitialize the framebuffer, e.g. when the surface has been resized.
    void resize(const surface_context & context, VkExtent2D extent);

    VkFramebuffer vk_framebuffer = VK_NULL_HANDLE;
    VkExtent2D extent;
    render_pass_ref render_pass;
    images::image_view_ref image_view;
    weak_device_ref device;
  };
}
