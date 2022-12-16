#pragma once

#include "strings/string_view.hpp"
#include "hzd/mem.hpp"

namespace j::mem {
  struct rw_buffer final {
    char * data_begin = nullptr;
    i32_t size = 0;
    i32_t capacity = 0;

    J_BOILERPLATE(rw_buffer, CTOR_CE, COPY_DEL)

    explicit rw_buffer(i32_t capacity);

    rw_buffer(rw_buffer && rhs) noexcept
      : data_begin(rhs.data_begin),
        size(rhs.size),
        capacity(rhs.capacity)
    {
      rhs.data_begin = nullptr;
      rhs.size = 0;
      rhs.capacity = 0;
      J_ASSERT(!capacity == !data_begin);
      J_ASSUME(size <= capacity);
    }

    rw_buffer & operator=(rw_buffer && rhs) noexcept {
      if (this != &rhs) {
        if (data_begin) {
          destroy();
        }
        data_begin = rhs.data_begin;
        size = rhs.size;
        capacity = rhs.capacity;
      }
      return *this;
    }


    J_INLINE_GETTER char * begin() noexcept {
      return data_begin;
    }

    J_INLINE_GETTER char * end() noexcept {
      return data_begin + size;
    }

    J_INLINE_GETTER const char * begin() const noexcept {
      return data_begin;
    }

    J_INLINE_GETTER const char * end() const noexcept {
      return data_begin + size;
    }

    [[nodiscard]] strings::const_string_view as_string_view() const noexcept {
      J_ASSUME(size <= capacity);
      return size ? strings::const_string_view{data_begin, size} : strings::const_string_view{};
    }

    J_INLINE_GETTER i32_t size_free() const noexcept {
      J_ASSUME(size <= capacity);
      return capacity - size;
    }

    i32_t write(const char * J_NOT_NULL chars, i32_t len) noexcept;

    J_ALWAYS_INLINE i32_t write(const strings::const_string_view & str) noexcept {
      return write(str.data(), str.size());
    }

    J_ALWAYS_INLINE void clear() noexcept {
      size = 0;
    }

    void destroy() noexcept {
      if (data_begin) {
        j::free(data_begin);
        data_begin = nullptr;
        size = 0;
        capacity = 0;
      }
    }

    ~rw_buffer() {
      destroy();
    }

    void remove_prefix(const char * J_NOT_NULL it) noexcept;

    J_ALWAYS_INLINE void remove_prefix(i32_t bytes) noexcept {
      return remove_prefix(data_begin + bytes);
    }
  };
}
