#pragma once

#include "rendering/vulkan/memory/free_list.hpp"
#include "rendering/vulkan/memory/allocation_result.hpp"

namespace j::rendering::vulkan::memory {
  class free_list_allocator;
  /// Allocator for blocks of varying size.
  class free_list_allocator final {
  public:
    J_BOILERPLATE(free_list_allocator, COPY_DEL, MOVE_DEL)
    explicit free_list_allocator(u32_t size) noexcept : m_free_list(size) { }

    /// Allocate [size] bytes.
    ///
    /// Throw if [size] is zero. If the allocation fails, the allocation result is failed but no
    /// exception is thrown.
    ///
    /// Not thread-safe.
    allocation_result allocate(u32_t size);

    /// Free allocation of size [size] at [offset].
    ///
    /// Partial frees are allowed.
    ///
    /// Throw if the allocation was not found.
    ///
    /// Not thread-safe.
    void free(u32_t offset, u32_t size);

    J_INLINE_GETTER u32_t free_size() const noexcept {
      return m_free_list.free_size;
    }

    J_INLINE_GETTER u32_t total_size() const noexcept {
      return m_free_list.total_size;
    }
  private:
    free_list m_free_list;
  };
}
