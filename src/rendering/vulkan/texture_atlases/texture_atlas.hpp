#pragma once

#include "rendering/texture_atlas/texture_atlas.hpp"
#include "rendering/vulkan/images/image_resource.hpp"

namespace j::rendering::vulkan {
  inline namespace texture_atlases {
    struct texture_atlas_description;
    struct texture_atlas {
      explicit texture_atlas(const device_context_base &,
                             const texture_atlas_description & desc);

      j::rendering::texture_atlas::texture_atlas atlas;
      images::image_ref image;
    };
  }
}
