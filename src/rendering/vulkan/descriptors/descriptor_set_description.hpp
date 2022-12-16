#pragma once

#include "rendering/vulkan/descriptors/descriptor_set_layout_resource.hpp"

namespace j::rendering::vulkan::descriptors {
  struct descriptor_set_description final {
    constexpr descriptor_set_description() noexcept = default;
    descriptor_set_description(descriptor_set_layout_ref layout) noexcept
      : layout(static_cast<descriptor_set_layout_ref &&>(layout))
    { J_ASSERT_NOT_NULL(this->layout); }

    descriptor_set_layout_ref layout;
  };
}
