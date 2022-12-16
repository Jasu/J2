#include "rendering/vulkan/rendering/render_pass.hpp"

#include "rendering/vulkan/rendering/render_pass_resource.hpp"
#include "rendering/vulkan/device.hpp"
#include "rendering/vulkan/utils.hpp"
#include "rendering/vulkan/resources/resource_definition.hpp"
#include "rendering/vulkan/context/surface_context.hpp"
#include "rendering/vulkan/context/device_context_base.hpp"
#include "rendering/vulkan/surfaces/surface.hpp"

namespace {
  const VkAttachmentReference color_attachment_ref{
    .attachment = 0,
    .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
  };

  const VkSubpassDescription subpass_description{
    .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
    .colorAttachmentCount = 1,
    .pColorAttachments = &color_attachment_ref,
  };
}

namespace j::rendering::vulkan::rendering {
  render_pass::render_pass(const surface_context & context)
    : device(context.device_ref())
  {
    const VkAttachmentDescription color_attachment{
      .format = context.surface->swapchain.vk_format.format,
      .samples = VK_SAMPLE_COUNT_1_BIT,
      .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
      .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
      .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
      .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
      .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
      .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
    };

    const VkRenderPassCreateInfo render_pass_info{
      .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
      .attachmentCount = 1,
      .pAttachments = &color_attachment,
      .subpassCount = 1,
      .pSubpasses = &subpass_description,
    };

    J_VK_CALL_CHECKED(vkCreateRenderPass, context.device().vk_device,
                      &render_pass_info, nullptr, &vk_render_pass);
  }

  render_pass::~render_pass() {
    vkDestroyRenderPass(device.get().vk_device, vk_render_pass, nullptr);
  }
  DEFINE_SURFACE_RESOURCE_DEFINITION(render_pass, void, void)
}
DEFINE_RESOURCE_TEMPLATES(j::rendering::vulkan::rendering::render_pass, void);
