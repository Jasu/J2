#pragma once

#include "rendering/vulkan/resources/resource_declaration.hpp"

namespace j::rendering::vulkan {
  inline namespace texture_atlases {
    struct texture_atlas;
    struct texture_atlas_description;
    DECLARE_RESOURCE(texture_atlas, texture_atlas_description, void);
  }
}
DECLARE_RESOURCE_TEMPLATES(j::rendering::vulkan::texture_atlas, j::rendering::vulkan::texture_atlas_description);
