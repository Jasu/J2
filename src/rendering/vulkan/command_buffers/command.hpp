#pragma once

#include "rendering/vulkan/state/condition_group.hpp"

namespace j::rendering::vulkan::inline command_buffers {
  /// Abstract base class for all Vulkan commands.
  class command : public state::condition_group {
  public:
    using state::condition_group::condition_group;
  };
}
