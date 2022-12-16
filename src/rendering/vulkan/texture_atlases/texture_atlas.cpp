#include "rendering/vulkan/texture_atlases/texture_atlas.hpp"

#include "rendering/vulkan/texture_atlases/texture_atlas_description.hpp"
#include "rendering/vulkan/texture_atlases/texture_atlas_resource.hpp"
#include "rendering/vulkan/images/image_description.hpp"
#include "rendering/vulkan/resources/resource_definition.hpp"

namespace j::rendering::vulkan {
  inline namespace texture_atlases {
    texture_atlas::texture_atlas(const device_context_base &,
                                 const texture_atlas_description & desc)
      : atlas(desc.image_description.size),
        image(desc.image_description)
    {
    }

    DEFINE_RESOURCE_DEFINITION(texture_atlas, texture_atlas_description, void)
  }
}
DEFINE_RESOURCE_TEMPLATES(j::rendering::vulkan::texture_atlas, j::rendering::vulkan::texture_atlas_description);
