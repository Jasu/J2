#include "containers/detail/chunk.hpp"

#include "exceptions/assert_lite.hpp"
#include "hzd/string.hpp"


namespace j::detail {
  void chunk::set_next(chunk * next) noexcept {
    J_ASSERT(m_capacity_bytes != 0, "Capacity cannot be zero.");
    *data<chunk *>(m_capacity_bytes) = next;
  }

  chunk * chunk::allocate_chunk(
    chunk * previous,
    chunk * next,
    u32_t capacity_bytes
  ) {
    J_ASSERT(capacity_bytes, "Tried to allocate an empty deque chunk.");

    sz_t size = sizeof(chunk)
      + capacity_bytes // Size of the actual contents
      + J_PTR_SZ; // Next pointer after the data.
    chunk * result = reinterpret_cast<chunk *>(::operator new(size));
    result->m_previous = previous;
    if (previous) {
      previous->set_next(result);
    }
    result->m_end_offset = 0;
    result->m_capacity_bytes = capacity_bytes;
    result->set_next(next);
    if (next) {
      next->m_previous = result;
    }
    return result;
  }

  chunk * chunk::release_head(chunk * c) noexcept {
    J_ASSUME_NOT_NULL(c);
    chunk * next = c->next();
    J_ASSERT(!next || next->prev() == c);
    ::operator delete(reinterpret_cast<void*>(c));
    if (next) {
      J_ASSERT(next->begin_offset() == 0, "Corrupted deque chunk list - begin offset != 0 for a non-head chunk.");
      next->m_previous = nullptr;
    }
    return next;
  }

  chunk * chunk::release_tail(chunk * c) noexcept {
    J_ASSERT_NOT_NULL(c);
    chunk * prev = c->prev();
    ::operator delete(reinterpret_cast<void*>(c));
    if (prev) {
      prev->set_next(nullptr);
    }
    return prev ;
  }

  void chunk::release_chunks_forward(chunk * chunk) noexcept {
    while((chunk = release_head(chunk)));
  }

  void chunk::copy_chunks_forward(
    const chunk * from,
    chunk * & head,
    chunk * & tail,
    bool memcpy_contents
  ) {
    if (!from) {
      head = tail = nullptr;
      return;
    }
    J_ASSERT(from->m_capacity_bytes, "Tried to copy an empty deque chunk.");
    J_ASSERT(!from->prev(), "Tried to copy a non-head deque chunk.");
    tail = head = allocate_chunk(nullptr, nullptr, from->m_capacity_bytes);
    head->m_begin_offset = from->m_begin_offset;
    try {
      do {
        tail->m_end_offset = from->m_end_offset;
        if (memcpy_contents) {
          memcpy(tail->data<void>(), from->data<void>(), from->m_capacity_bytes);
        }
        from = from->next();
        if (from) {
          J_ASSERT(from->m_capacity_bytes, "Tried to copy an empty deque chunk.");
          tail = allocate_chunk(tail, nullptr, from->m_capacity_bytes);
        }
      } while (from);
    } catch (...) {
      release_chunks_forward(head);
      head = tail = nullptr;
      throw;
    }
  }

  u32_t chunk_size_forward(const chunk * head, u32_t element_size) noexcept {
    if (!head) {
      return 0;
    }
    u32_t size = -head->begin_offset();
    J_ASSERT(head->begin_offset() % element_size == 0, "Head begin offset did not match.");
    J_ASSERT(head->begin_offset() < head->end_offset(), "Head begin offset was at or past end offset.");
    while (head) {
      J_ASSERT(head->end_offset(), "Deque chunk was empty.");
      J_ASSERT(head->end_offset() <= head->capacity_bytes(), "Deque chunk end offset was past capacity.");
      J_ASSERT(head->end_offset() % element_size == 0, "End offset did not match.");
      size += head->end_offset();
      head = head->next();
    }
    return size / element_size;
  }
}
