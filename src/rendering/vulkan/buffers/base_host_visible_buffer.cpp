#include "rendering/vulkan/buffers/base_host_visible_buffer.hpp"
#include "hzd/mem.hpp"

namespace j::rendering::vulkan::buffers {
  void base_host_visible_buffer::zero() noexcept {
    ::j::memzero(m_mapping, size_bytes);
  }

  base_host_visible_buffer::~base_host_visible_buffer() {
    unmap_memory();
  }

  void base_host_visible_buffer::copy_to_buffer(VkDeviceSize offset, const void * source, VkDeviceSize size) {
    raw_buffer::assert_range_in_bounds(offset, size);
    raw_buffer::assert_region_valid(source, size);
    ::j::memcpy(m_mapping + offset, source, size);
  }

  void base_host_visible_buffer::copy_to_buffer(const void * source, VkDeviceSize size) {
    raw_buffer::assert_region_valid(source, size);
    ::j::memcpy(m_mapping, source, size);
  }
}
