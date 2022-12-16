#pragma once

#include "rendering/vulkan/images/image_description.hpp"

namespace j::rendering::vulkan::inline texture_atlases {
  struct texture_atlas_description final {
    J_BOILERPLATE(texture_atlas_description, CTOR_CE, COPY_CE, MOVE_CE)

    vulkan::images::image_description image_description;

    template<typename... Args>
    J_ALWAYS_INLINE constexpr texture_atlas_description(Args && ... args)
      : image_description(static_cast<Args &&>(args)...)
    { }

    J_ALWAYS_INLINE constexpr texture_atlas_description(const vulkan::images::image_description & image_description) noexcept
      : image_description(image_description)
    { }
  };
}
