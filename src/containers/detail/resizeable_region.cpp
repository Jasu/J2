#include "containers/detail/resizeable_region.hpp"

namespace j::detail {
  resizeable_byte_region::resizeable_byte_region(const resizeable_byte_region & other, u32_t size_bytes, u32_t capacity_bytes)
    : ptr(capacity_bytes ? ::j::allocate(capacity_bytes) : nullptr)
  {
    J_ASSERT(size_bytes <= capacity_bytes);
    if (size_bytes) {
      J_ASSERT_NOT_NULL(other.ptr);
      ::j::memcpy(ptr, other.ptr, size_bytes);
    }
  }


  resizeable_byte_region::resizeable_byte_region(const void * J_NOT_NULL from, u32_t size_bytes, u32_t capacity_bytes)
    : ptr(capacity_bytes ? ::j::allocate(capacity_bytes) : nullptr)
  {
    J_ASSERT(size_bytes <= capacity_bytes);
    if (size_bytes) {
      ::j::memcpy(ptr, from, size_bytes);
    }
  }

  void resizeable_byte_region::zero(void * J_NOT_NULL begin, u32_t size_bytes) noexcept {
    J_ASSERT(begin >= ptr);
    ::j::memzero(begin, size_bytes);
  }
}
