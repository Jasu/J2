#include "mem/rw_buffer.hpp"
#include "hzd/mem.hpp"

namespace j::mem {
  rw_buffer::rw_buffer(i32_t capacity)
    : data_begin(capacity ? (char*)::j::allocate(capacity) : nullptr),
      capacity(capacity)
  {
    if (capacity) {
      ::j::memzero(data_begin, capacity);
    }
  }

  i32_t rw_buffer::write(const char * J_NOT_NULL chars, i32_t len) noexcept {
    len = min(len, size_free());
    if (J_LIKELY(len)) {
      J_ASSUME_NOT_NULL(data_begin);
      ::j::memcpy(data_begin + size, chars, len);
      size += len;
      J_ASSERT(size <= capacity);
    }
    return len;
  }

  void rw_buffer::remove_prefix(const char * J_NOT_NULL it) noexcept {
    J_ASSUME_NOT_NULL(data_begin);
    J_ASSUME(it >= data_begin);
    i32_t sz = it - data_begin;
    J_ASSUME(sz <= size);
    size -= sz;
    ::j::memmove(data_begin, it, size);
    ::j::memzero(data_begin + size, sz);
  }
}
