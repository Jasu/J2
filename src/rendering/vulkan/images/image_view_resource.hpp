#pragma once

#include "rendering/vulkan/resources/resource_declaration.hpp"

namespace j::rendering::vulkan::images {
  struct image_view;
  class image_view_description;
  DECLARE_RESOURCE(image_view, image_view_description, void);
}
DECLARE_RESOURCE_TEMPLATES(j::rendering::vulkan::images::image_view, j::rendering::vulkan::images::image_view_description);
