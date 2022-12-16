#pragma once

#include "hzd/types.hpp"

namespace j::mem {
  struct bump_pool_node;
  struct bump_pool_checkpoint final {
    bump_pool_node * node = nullptr;
    u32_t size = 0U;
  };
  class bump_pool;
}
