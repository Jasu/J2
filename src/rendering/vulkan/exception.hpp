#pragma once

#include "exceptions/exceptions.hpp"
#include <vulkan/vulkan.h>

namespace j::rendering::vulkan {
  /// Signals that the surface is out of date, and the command buffer should be recreated.
  class error_surface_out_of_date : public j::exceptions::exception {};

  extern const tags::tag_definition<VkResult> vulkan_result;
}
