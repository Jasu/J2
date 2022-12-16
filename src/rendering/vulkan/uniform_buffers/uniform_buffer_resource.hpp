#pragma once

#include "rendering/vulkan/resources/resource_declaration.hpp"

namespace j::rendering::vulkan {
  inline namespace uniform_buffers {
    class uniform_buffer;
    struct uniform_buffer_description;
    DECLARE_RESOURCE(uniform_buffer, uniform_buffer_description, void);
  }
}
DECLARE_RESOURCE_TEMPLATES(j::rendering::vulkan::uniform_buffer,
                           j::rendering::vulkan::uniform_buffer_description);
