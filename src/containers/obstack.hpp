#pragma once

#include "containers/obstack_fwd.hpp"
#include "hzd/mem.hpp"

namespace j::detail {
  struct obstack_chunk {
    u32_t size;
    obstack_chunk * next;
    char data[1];
  };

  J_RETURNS_NONNULL obstack_chunk * allocate_obstack_chunk(u32_t chunk_size, obstack_chunk * next = nullptr);

  obstack_chunk * release_obstack_chunk(obstack_chunk * J_NOT_NULL chunk) noexcept;
}

namespace j::inline containers {
  template<typename T, typename Chunk>
  T & obstack_iterator<T, Chunk>::operator*() const noexcept {
    return reinterpret_cast<T*>(&chunk->data)[i];
  }

  template<typename T, typename Chunk>
  J_RETURNS_NONNULL T * obstack_iterator<T, Chunk>::operator->() const noexcept {
    return &reinterpret_cast<T*>(&chunk->data)[i];
  }

  template<typename T, typename Chunk>
  obstack_iterator<T, Chunk> & obstack_iterator<T, Chunk>::operator++() noexcept {
    if (++i == chunk->size) {
      chunk = chunk->next, i = 0;
    }
    return *this;
  }

  template<typename T, typename Chunk>
  obstack_iterator<T, Chunk> obstack_iterator<T, Chunk>::operator++(int) noexcept {
    obstack_iterator result{chunk, i};
    operator++();
    return result;
  }

  template<typename T, int Num>
  obstack<T, Num> & obstack<T, Num>::operator=(obstack && rhs) noexcept {
    if (this != &rhs) {
      clear();
      m_head = rhs.m_head;
      rhs.m_head = nullptr;
    }
    return *this;
  }

  template<typename T, int Num>
  T & obstack<T, Num>::push(const T & t) {
    return emplace(t);
  }

  template<typename T, int Num>
  T & obstack<T, Num>::push(T && t) {
    return emplace(static_cast<T &&>(t));
  }

  template<typename T, int Num>
  template<typename... Args>
  T & obstack<T, Num>::emplace(Args && ... args) {
    if (!m_head || m_head->size == Num) {
      m_head = detail::allocate_obstack_chunk(sizeof(T) * Num, m_head);
    }
    auto ptr = ::new (reinterpret_cast<T*>(&m_head->data) + m_head->size) T(static_cast<Args &&>(args)...);
    ++m_head->size;
    return *ptr;
  }

  template<typename T, int Num>
  u32_t obstack<T, Num>::size() const noexcept {
    u32_t result = 0;
    for (auto cur = m_head; cur; cur = cur->next) {
      result += cur->size;
    }
    return result;
  }

  template<typename T, int Num>
  T & obstack<T, Num>::top() {
    return *(reinterpret_cast<T*>(&m_head->data) + m_head->size - 1);
  }

  template<typename T, int Num>
  void obstack<T, Num>::pop() noexcept {
    if (m_head) {
      reinterpret_cast<T*>(&m_head->data)[--m_head->size].~T();
      if (m_head->size == 0) {
        m_head = detail::release_obstack_chunk(m_head);
      }
    }
  }

  template<typename T, int Num>
  void obstack<T, Num>::clear() noexcept {
    for (auto cur = m_head; cur; cur = detail::release_obstack_chunk(cur)) {
      for (i32_t i = cur->size - 1; i >= 0; --i) {
        reinterpret_cast<T*>(&cur->data)[i].~T();
      }
    }
    m_head = nullptr;
  }

  template<typename T, int Num>
  J_NO_EXPLICIT void obstack<T, Num>::copy_initialize(detail::obstack_chunk * c) {
    for (detail::obstack_chunk * cur = nullptr; c; c = c->next) {
      {
        detail::obstack_chunk * new_chunk = detail::allocate_obstack_chunk(sizeof(T) * Num);
        if (cur) {
          cur->next = new_chunk;
        } else {
          m_head = new_chunk;
        }
        cur = new_chunk;
      }

      for (cur->size = 0; cur->size < c->size; ++cur->size) {
        ::new (reinterpret_cast<T*>(&cur->data) + cur->size) T(
          *(reinterpret_cast<T*>(&c->data) + cur->size)
        );
      }
    }
  }
}

#define J_DEFINE_EXTERN_OBSTACK(TYPE)                                                         \
  template class j::containers::obstack<TYPE>;                                                \
  template class j::containers::obstack_iterator<TYPE, j::detail::obstack_chunk>; \
  template class j::containers::obstack_iterator<const TYPE, const j::detail::obstack_chunk>
