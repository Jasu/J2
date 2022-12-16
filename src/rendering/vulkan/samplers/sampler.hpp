#pragma once

#include <vulkan/vulkan.h>

#include "rendering/vulkan/device_resource.hpp"

namespace j::rendering::vulkan::samplers {
  struct sampler final {
    explicit sampler(const device_context_base & context);

    ~sampler();

    sampler(sampler &&) = delete;
    sampler & operator=(sampler &&) = delete;

    VkSampler vk_sampler;
    weak_device_ref device;
  };
}
