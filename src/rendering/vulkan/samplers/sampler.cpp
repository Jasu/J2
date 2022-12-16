#include "rendering/vulkan/samplers/sampler.hpp"

#include "rendering/vulkan/context/device_context_base.hpp"
#include "rendering/vulkan/device.hpp"
#include "rendering/vulkan/utils.hpp"
#include "rendering/vulkan/resources/resource_definition.hpp"

namespace j::rendering::vulkan::samplers {
  namespace {
    const VkSamplerCreateInfo create_info{
      .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
      .magFilter = VK_FILTER_LINEAR,
      .minFilter = VK_FILTER_LINEAR,
      .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
      .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
      .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
      .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
      .mipLodBias = 0.0f,
      .anisotropyEnable = VK_FALSE,
      .maxAnisotropy = 1.0f,
      .compareOp = VK_COMPARE_OP_NEVER,
      .minLod = 0.0f,
      .maxLod = 0.0f,
      .borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE,
    };
  }

  sampler::sampler(const device_context_base & context)
    : device(context.device_ref())
  {
    J_VK_CALL_CHECKED(vkCreateSampler, context.device().vk_device, &create_info, nullptr, &vk_sampler);
  }

  sampler::~sampler() {
    if (auto dev = device.lock()) {
      vkDestroySampler(dev.get().vk_device, vk_sampler, nullptr);
    }
  }
  DEFINE_RESOURCE_DEFINITION(sampler, void, void)
}
DEFINE_RESOURCE_TEMPLATES(j::rendering::vulkan::samplers::sampler, void);
