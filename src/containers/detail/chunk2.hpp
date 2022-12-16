#pragma once

#include "exceptions/assert_lite.hpp"

namespace j::detail {
  class simple_chunk final {
  public:
    J_INLINE_GETTER_NONNULL void * raw() noexcept { return this + 1; }

    J_INLINE_GETTER_NONNULL const void * raw() const noexcept { return this + 1; }

    template<typename T>
    J_INLINE_GETTER_NONNULL T * data(u32_t byte_offset) noexcept {
      return add_bytes<T*>(this + 1, byte_offset);
    }

    template<typename T>
    J_INLINE_GETTER_NONNULL const T * data(u32_t byte_offset) const noexcept {
      return add_bytes<const T*>(this + 1, byte_offset);
    }

    static simple_chunk * allocate_chunk(simple_chunk * previous, simple_chunk * next, u32_t capacity_bytes);

    [[nodiscard]] static u32_t size_forward(const simple_chunk * cur) noexcept;

    static void release_forward(simple_chunk * c) noexcept;

    static simple_chunk * release_tail(simple_chunk * J_NOT_NULL c) noexcept;

    static void copy_chunks_forward(const simple_chunk * from,
                                    simple_chunk ** J_NOT_NULL head,
                                    simple_chunk ** J_NOT_NULL const tail,
                                    const bool memcpy_contents);

    simple_chunk * previous = nullptr;
    simple_chunk * next = nullptr;
    u32_t size_bytes = 0U;
    u32_t capacity_bytes = 0U;
  };
}
