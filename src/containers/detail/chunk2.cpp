#include "containers/detail/chunk2.hpp"
#include "hzd/mem.hpp"
#include "hzd/string.hpp"

namespace j::detail {
  [[nodiscard]] u32_t simple_chunk::size_forward(const simple_chunk * cur) noexcept {
    u32_t result = 0U;
    for (; cur; cur = cur->next) {
      result += cur->size_bytes;
    }
    return result;
  }
  simple_chunk * simple_chunk::release_tail(simple_chunk * J_NOT_NULL c) noexcept {
    simple_chunk * const prev = c->previous;
    if (prev) {
      prev->next = nullptr;
    }
    ::j::free(c);
    return prev;
  }

  void simple_chunk::copy_chunks_forward(const simple_chunk * from,
                                    simple_chunk ** J_NOT_NULL head,
                                    simple_chunk ** J_NOT_NULL const tail,
                                    const bool memcpy_contents)
  {
    if (!from) {
      *head = *tail = nullptr;
      return;
    }
    simple_chunk * cur = nullptr;
    do {
      const u32_t cap = from->capacity_bytes;
      cur = *head = allocate_chunk(cur, nullptr, cap);
      cur->size_bytes = from->size_bytes;
      if (memcpy_contents) {
        ::j::memcpy(cur + 1, from + 1, cap);
      }
      head = &cur->next;
      from = from->next;
    } while (from);
    *tail = cur;
  }

  void simple_chunk::release_forward(simple_chunk * c) noexcept {
    while (c) {
      simple_chunk * const n = c->next;
      ::j::free(c);
      c = n;
    }
  }
  simple_chunk * simple_chunk::allocate_chunk(

      simple_chunk * previous,
      simple_chunk * next,
      u32_t capacity_bytes)
    {
      return ::new (::j::allocate(sizeof(simple_chunk) + capacity_bytes)) simple_chunk{
        previous,
        next,
        0U,
        capacity_bytes
      };
    }

}
