#include "rendering/vulkan/buffers/suballocation.hpp"

template class j::mem::pool<j::rendering::vulkan::buffers::generic_suballocation>;

namespace j::rendering::vulkan::buffers {
  suballocation_pool_t g_suballocation_pool;
}
