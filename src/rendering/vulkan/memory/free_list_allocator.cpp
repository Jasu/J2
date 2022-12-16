#include "rendering/vulkan/memory/free_list_allocator.hpp"
#include "rendering/vulkan/utils.hpp"

namespace j::rendering::vulkan::memory {
  void allocation_result::throw_unsuccessful() {
    J_VK_THROW("Tried to access an unsuccessful allocation result.");
  }

  allocation_result free_list_allocator::allocate(u32_t size) {
    J_ASSERT_NOT_NULL(size);
    const auto end = m_free_list.by_offset.end();
    for (auto it = m_free_list.by_offset.begin(); it != end; ++it) {
      if (it->size >= size) {
        const auto offset = it->offset;
        m_free_list.allocate_from_start_of(it, size);
        return allocation_result(offset);
      }
    }
    return allocation_result();
  }

  void free_list_allocator::free(u32_t offset, u32_t size) {
    J_ASSERT_NOT_NULL(size);
    const auto end_offset = offset + size;
    J_VK_ASSERT(offset < m_free_list.total_size && end_offset <= m_free_list.total_size,
                "Tried to free out of bounds.");
    auto it = m_free_list.by_offset.begin();
    for (const auto end = m_free_list.by_offset.end(); it != end && it->offset < end_offset; ++it) { }
    m_free_list.free_before(it, offset, size);
  }
}
