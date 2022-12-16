#include "containers/ptr_set.hpp"
#include "algo/copy.hpp"

namespace j::detail {
  void ptr_set_add(iptr_t * J_NOT_NULL data, iptr_t ptr) {
    iptr_t cur = *data;
    J_ASSERT(cur && ptr);
    if (cur > 0) {
      ptr_chunk * chunk = reinterpret_cast<ptr_chunk*>(j::allocate(sizeof(ptr_chunk) + sizeof(iptr_t) * 4));
      chunk->size = 2U;
      chunk->capacity = 4U;
      chunk->begin()[0] = cur < ptr ? cur : ptr;
      chunk->begin()[1] = cur < ptr ? ptr : cur;
      *data = -reinterpret_cast<iptr_t>(chunk);
      return;
    }

    ptr_chunk * chunk = reinterpret_cast<ptr_chunk*>(-cur);

    u32_t insert_point = 0;
    const u32_t sz = chunk->size;
    iptr_t * p = chunk->begin();
    for (; insert_point < sz; ++insert_point, ++p) {
      if (*p == ptr) {
        return;
      } else if (*p > ptr) {
        break;
      }
    }

    if (chunk->size == chunk->capacity) {
      chunk = reinterpret_cast<ptr_chunk*>(
      j::reallocate((void*)chunk,
                    sizeof(ptr_chunk) + sizeof(iptr_t) * (chunk->capacity + 2)));
      chunk->capacity += 2;
      *data = -reinterpret_cast<iptr_t>(chunk);
    }

    auto insert_ptr = chunk->begin() + insert_point;
    if (insert_point < sz) {
      algo::move_forward(insert_ptr + 1, insert_ptr, sz - insert_point);
    }
    *insert_ptr = ptr;
    ++chunk->size;
  }

  void ptr_array_push_back(iptr_t * J_NOT_NULL data, iptr_t ptr) {
    iptr_t cur = *data;
    if (cur > 0) {
      ptr_chunk * chunk = reinterpret_cast<ptr_chunk*>(j::allocate(sizeof(ptr_chunk) + sizeof(iptr_t) * 4));
      chunk->size = 2U;
      chunk->capacity = 4U;
      chunk->begin()[0] = cur;
      chunk->begin()[1] = ptr;
      *data = -reinterpret_cast<iptr_t>(chunk);
      return;
    }

    ptr_chunk * chunk = reinterpret_cast<ptr_chunk*>(-cur);
    if (chunk->size == chunk->capacity) {
      chunk = reinterpret_cast<ptr_chunk*>(
      j::reallocate((void*)chunk,
                    sizeof(ptr_chunk) + sizeof(iptr_t) * (chunk->capacity + 2)));
      chunk->capacity += 2;
      *data = -reinterpret_cast<iptr_t>(chunk);
    }
    *chunk->end() = ptr;
    ++chunk->size;
  }

  bool ptr_container_impl::do_contains(iptr_t ptr) const noexcept {
    const ptr_chunk * chunk = reinterpret_cast<const ptr_chunk*>(-m_ptr);
    J_ASSUME_NOT_NULL(chunk);
    for (const iptr_t * cur = chunk->begin(), * const end = cur + chunk->size; cur != end; ++cur) {
      if (*cur == ptr) {
        return true;
      }
      /*
      else if (*cur > ptr) {
        return false;
      }
      */
    }
    return false;
  }

  u32_t ptr_container_impl::do_index_of(iptr_t ptr) const noexcept {
    const ptr_chunk * chunk = reinterpret_cast<const ptr_chunk*>(-m_ptr);
    J_ASSUME_NOT_NULL(chunk);
    const iptr_t * const begin = chunk->begin();
    for (u32_t i = 0, sz = chunk->size; i < sz; ++i) {
      if (begin[i] == ptr) {
        return i;
      }
    }
    return npos;
  }

  void ptr_container_impl::dup() {
    const ptr_chunk * chunk = reinterpret_cast<const ptr_chunk*>(-m_ptr);
    J_ASSUME_NOT_NULL(chunk);
    ptr_chunk * new_chunk = reinterpret_cast<ptr_chunk*>(j::allocate(sizeof(ptr_chunk) + sizeof(iptr_t) * chunk->size));
    ::j::memcpy(new_chunk, chunk, sizeof(ptr_chunk) + sizeof(iptr_t) * chunk->size);
    new_chunk->capacity = chunk->size;
    m_ptr = -reinterpret_cast<iptr_t>(new_chunk);
  }

  void ptr_container_impl::iptr_remove(iptr_t ptr) noexcept {
    if (!m_ptr) {
      return;
    } else if (m_ptr > 0) {
      if (m_ptr == ptr) {
        m_ptr = 0;
      }
      return;
    } else {
      ptr_chunk * chunk = reinterpret_cast<ptr_chunk*>(-m_ptr);
      J_ASSUME_NOT_NULL(chunk);
      for (iptr_t * cur = chunk->begin(), * const end = cur + chunk->size; cur != end; ++cur) {
        if (*cur == ptr) {
          ::j::memmove(cur, cur + 1, (end - cur - 1) * sizeof(iptr_t));
          --chunk->size;
          return;
        }
      }
    }
  }
}
