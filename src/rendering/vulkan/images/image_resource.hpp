#pragma once

#include "rendering/vulkan/resources/resource_declaration.hpp"

namespace j::rendering::vulkan::images {
  struct image;
  struct image_description;
  struct image_state;
  DECLARE_RESOURCE(image, image_description, image_state);
}
DECLARE_RESOURCE_TEMPLATES(j::rendering::vulkan::images::image, j::rendering::vulkan::images::image_description);
