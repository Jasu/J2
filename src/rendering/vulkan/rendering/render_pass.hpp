#pragma once

#include <vulkan/vulkan.h>
#include "rendering/vulkan/device_resource.hpp"

namespace j::rendering::vulkan::rendering {
  struct render_pass_description;

  /// Render pass describes what kinds of framebuffers are bound when executing a pipeline.
  ///
  /// The specific attachments are specified in a framebuffer.
  class render_pass {
  public:
    render_pass(const surface_context & context);

    ~render_pass();

    render_pass(const render_pass &) = delete;
    render_pass & operator=(const render_pass &) = delete;

    VkRenderPass vk_render_pass;
    device_ref device;
  };
}
