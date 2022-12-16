#pragma once

#include "containers/deque_fwd.hpp"
#include "hzd/mem.hpp"
#include "hzd/type_traits.hpp"

namespace j::detail {
  template<typename T>
  J_ALWAYS_INLINE void copy_initialize_deque(deque<T> & to, const deque<T> & from) {
    constexpr bool is_trivial = j::is_trivially_copyable_v<T>;
    chunk::copy_chunks_forward(from.m_head, to.m_head, to.m_tail, is_trivial);
    if constexpr (!is_trivial) {
      const chunk * from_chunk = from.m_head;
      for (chunk *to_chunk = to.m_head;
           to_chunk;
           to_chunk = to_chunk->next(),
             from_chunk = from_chunk->next())
      {
        for (u32_t offset = to_chunk->begin_offset();
             offset != to_chunk->end_offset();
             offset += sizeof(T)
          ) {
          ::new (to_chunk->data<void>(offset)) T(*from_chunk->data<T>(offset));
        }
      }
    }
  }

  inline void * begin_deque_emplace_front(chunk * & head, chunk * & tail, u32_t sz) {
    if (!head || !head->begin_offset()) {
      head = detail::chunk::allocate_chunk(nullptr, head, sz * 8UL);
      head->set_begin_offset(head->capacity_bytes());
      head->set_end_offset(head->capacity_bytes());
      if (!tail) {
        tail = head;
      }
    }
    return head->data<void>(head->begin_offset() - sz);
  }

  inline void * begin_deque_emplace_back(chunk * & head, chunk * & tail, u32_t sz) {
    if (!tail || tail->capacity_bytes() == tail->end_offset()) {
      tail = detail::chunk::allocate_chunk(tail, nullptr, sz * 8UL);
      if (!head) {
        head = tail;
      }
    }
    return tail->data<void>(tail->end_offset());
  }
}

namespace j::inline containers {
  template<typename T, typename C>
  deque_iterator<T, C> & deque_iterator<T, C>::operator--() noexcept {
    if (byte_offset == 0) {
      chunk = chunk->prev();
      if (chunk) {
        byte_offset = byte_offset_max = chunk->capacity_bytes();
      }
    }
    byte_offset -= sizeof(T);
    return *this;
  }

  template<typename T>
  deque<T> & deque<T>::operator=(deque && rhs) noexcept {
    if (J_LIKELY(this != &rhs)) {
      clear();
      m_head = rhs.m_head, m_tail = rhs.m_tail;
      rhs.m_head = nullptr, rhs.m_tail = nullptr;
    }
    return *this;
  }


  template<typename T>
  T & deque<T>::push_back(T && value) {
    return emplace_back(static_cast<T &&>(value));
  }

  template<typename T>
  T & deque<T>::push_front(T && value) {
    return emplace_front(static_cast<T &&>(value));
  }

  template<typename T>
  T & deque<T>::push_back(const T & value) {
    return emplace_back(value);
  }

  template<typename T>
  T & deque<T>::push_front(const T & value) {
    return emplace_front(value);
  }

  template<typename T>
  u32_t deque<T>::size() const noexcept {
    return detail::chunk_size_forward(m_head, sizeof(T));
  }

  template<typename T>
  void deque<T>::clear() noexcept {
    if (!m_head) {
      return;
    }
    if constexpr (!j::is_trivially_destructible_v<T>) {
      for (auto chunk = m_head; chunk; chunk = chunk->next()) {
        for (u32_t offset = chunk->begin_offset(); offset != chunk->end_offset(); offset += sizeof(T)) {
          chunk->data<T>(offset)->~T();
        }
      }
    }
    detail::chunk::release_chunks_forward(m_head);
    m_head = m_tail = nullptr;
  }

  template<typename T>
  typename deque<T>::iterator deque<T>::begin() noexcept {
    return {
      m_head,
      m_head ? m_head->begin_offset() : 0U,
      m_head ? m_head->end_offset() : 0U
    };
  }
}

#define J_DEFINE_EXTERN_DEQUE(...) template class j::deque<__VA_ARGS__>
