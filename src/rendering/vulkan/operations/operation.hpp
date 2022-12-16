#pragma once

#include "rendering/vulkan/state/condition_group.hpp"

namespace j::rendering::vulkan::inline context {
  struct render_context;
}

namespace j::rendering::vulkan::operations {
  class operation : public state::condition_group {
  public:
    using state::condition_group::condition_group;
  };
}
