#include "rendering/vulkan/rendering/framebuffer.hpp"

#include "rendering/vulkan/rendering/render_pass.hpp"
#include "rendering/vulkan/device.hpp"
#include "rendering/vulkan/utils.hpp"
#include "rendering/vulkan/context/surface_context.hpp"
#include "rendering/vulkan/context/device_context_base.hpp"
#include "rendering/vulkan/images/image_view.hpp"

namespace j::rendering::vulkan::rendering {
  void framebuffer::resize(const surface_context & context, VkExtent2D extent) {
    this->extent = extent;
    const VkFramebufferCreateInfo framebuffer_info{
      .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
      .renderPass = context.get(render_pass).vk_render_pass,
      .attachmentCount = 1,
      .pAttachments = &context.get(image_view).vk_image_view,
      .width = extent.width,
      .height = extent.height,
      .layers = 1,
    };

    VkDevice vk_device = context.device().vk_device;
    if (vk_framebuffer) {
      vkDestroyFramebuffer(vk_device, vk_framebuffer, nullptr);
      vk_framebuffer = VK_NULL_HANDLE;
    }
    J_VK_CALL_CHECKED(vkCreateFramebuffer, vk_device, &framebuffer_info,
                      nullptr, &vk_framebuffer);
  }

  framebuffer::framebuffer(const surface_context & context,
                           VkExtent2D vk_extent,
                           render_pass_ref render_pass,
                           images::image_view_ref image_view)
    : render_pass(static_cast<render_pass_ref &&>(render_pass)),
      image_view(static_cast<images::image_view_ref &&>(image_view)),
      device(context.device_ref())
  {
    resize(context, vk_extent);
  }

  framebuffer::~framebuffer() {
    if (auto dev = device.lock()) {
      vkDestroyFramebuffer(dev.get().vk_device, vk_framebuffer, nullptr);
    }
  }
}
