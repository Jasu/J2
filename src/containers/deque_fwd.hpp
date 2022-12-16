#pragma once

#include "hzd/utility.hpp"
#include "containers/detail/chunk.hpp"

namespace j::inline containers {
  template<typename T> class deque;
}

namespace j::detail {
  class chunk;
  template<typename T>
  J_ALWAYS_INLINE void copy_initialize_deque(deque<T> & to, const deque<T> & from);

  void * begin_deque_emplace_front(chunk * & head, chunk * & tail, u32_t sz);
  void * begin_deque_emplace_back(chunk * & head, chunk * & tail, u32_t sz);
}

namespace j::inline containers {
  template<typename T, typename Chunk>
  class deque_iterator final {
  public:
    J_INLINE_GETTER_NONNULL J_NO_EXPLICIT T * operator->() const noexcept {
      return chunk->template data<T>(byte_offset);
    }

    J_INLINE_GETTER J_NO_EXPLICIT T & operator*() const noexcept {
      return *operator->();
    }

    J_ALWAYS_INLINE J_NO_EXPLICIT deque_iterator & operator++() noexcept {
      byte_offset += sizeof(T);
      if (byte_offset == byte_offset_max) {
        byte_offset = 0;
        chunk = chunk->next();
        byte_offset_max = chunk ? chunk->end_offset() : 0;
      }
      return *this;
    }

    J_ALWAYS_INLINE J_NO_EXPLICIT deque_iterator operator++(int) noexcept {
      deque_iterator result(*this);
      operator++();
      return result;
    }

    J_NO_EXPLICIT deque_iterator & operator--() noexcept;

    J_ALWAYS_INLINE J_NO_EXPLICIT deque_iterator operator--(int) noexcept {
      deque_iterator result(*this);
      operator--();
      return result;
    }

    J_ALWAYS_INLINE J_NO_EXPLICIT bool operator==(const deque_iterator &) const noexcept = default;

    J_NO_EXPLICIT operator deque_iterator<const T, const Chunk>() const noexcept {
      return {chunk, byte_offset, byte_offset_max};
    }

    Chunk * chunk;
    u32_t byte_offset;
    u32_t byte_offset_max;
  };

  template<typename T>
  class deque final {
  public:
    using iterator J_NO_DEBUG_TYPE = deque_iterator<T, detail::chunk>;
    using const_iterator J_NO_DEBUG_TYPE = deque_iterator<const T, const detail::chunk>;

    J_ALWAYS_INLINE J_NO_EXPLICIT constexpr deque() noexcept = default;

    /// Move-construct from rhs.
    J_ALWAYS_INLINE J_NO_EXPLICIT deque(deque && rhs) noexcept
      : m_head(rhs.m_head),
        m_tail(rhs.m_tail)
    {
      rhs.m_head = nullptr, rhs.m_tail = nullptr;
    }

    /// Move-assign from rhs.
    deque & operator=(deque && rhs) noexcept;

    /// Copy-construct from rhs.
    J_NO_EXPLICIT deque(const deque & rhs);
    /// Copy-assign from rhs.
    J_NO_EXPLICIT deque & operator=(const deque & rhs);


    /// Emplace an item as the last item of the deque.
    template<typename... Args>
    J_NO_EXPLICIT J_NO_DEBUG T & emplace_back(Args && ... args) {
      T * result = ::new (detail::begin_deque_emplace_back(m_head, m_tail, sizeof(T))) T{static_cast<Args &&>(args)...};
      return m_tail->set_end_offset(m_tail->end_offset() + sizeof(T)), *result;
    }

    /// Emplace an item as the last first of the deque.
    template<typename... Args>
    J_NO_EXPLICIT J_NO_DEBUG T & emplace_front(Args && ... args) {
      T * result = ::new (detail::begin_deque_emplace_front(m_head, m_tail, sizeof(T))) T{static_cast<Args &&>(args)...};
      return m_head->set_begin_offset(m_head->begin_offset() - sizeof(T)), *result;
    }

    /// Move-insert value as the last item of the deque.
    T & push_back(T && value);
    /// Move-insert value as the first item of the deque.
    T & push_front(T && value);

    /// Copy-insert value as the last item of the deque.
    T & push_back(const T & value);
    /// Copy-insert value as the first item of the deque.
    T & push_front(const T & value);

    J_NO_EXPLICIT J_ALWAYS_INLINE T & front() noexcept {
      return *m_head->data<T>(m_head->begin_offset());
    }
    J_NO_EXPLICIT J_ALWAYS_INLINE const T & front() const noexcept {
      return *m_head->data<const T>(m_head->begin_offset());
    }
    J_NO_EXPLICIT J_ALWAYS_INLINE T & back() noexcept {
      return *m_tail->data<T>(m_tail->end_offset() - sizeof(T));
    }
    J_NO_EXPLICIT J_ALWAYS_INLINE const T & back() const noexcept {
      return *m_tail->data<const T>(m_tail->end_offset() - sizeof(T));
    }

    J_NO_EXPLICIT void pop_front() noexcept;
    J_NO_EXPLICIT void pop_back() noexcept;

    J_INLINE_GETTER J_NO_EXPLICIT bool empty() const noexcept {
      return !m_head;
    }
    u32_t size() const noexcept;

    void clear() noexcept;

    iterator begin() noexcept;
    J_INLINE_GETTER J_NO_EXPLICIT const_iterator begin() const noexcept {
      return const_cast<deque*>(this)->begin();
    }

    J_INLINE_GETTER J_NO_EXPLICIT constexpr iterator end() noexcept {
      return iterator{nullptr, 0U, 0U};
    }
    J_INLINE_GETTER J_NO_EXPLICIT constexpr const_iterator end() const noexcept {
      return const_iterator{nullptr, 0U, 0U};
    }

    J_ALWAYS_INLINE J_NO_EXPLICIT ~deque() {
      clear();
    }
  private:
    detail::chunk * m_head = nullptr;
    detail::chunk * m_tail = nullptr;

    friend void detail::copy_initialize_deque<T>(deque &, const deque &);
  };

  template<typename T>
  J_NO_EXPLICIT J_ALWAYS_INLINE deque<T>::deque(const deque & rhs) {
    detail::copy_initialize_deque(*this, rhs);
  }

  template<typename T>
  J_NO_EXPLICIT J_ALWAYS_INLINE deque<T> & deque<T>::operator=(const deque & rhs) {
    if (J_LIKELY(this != &rhs)) {
      clear();
      detail::copy_initialize_deque(*this, rhs);
    }
    return *this;
  }


  template<typename T>
  J_NO_EXPLICIT J_ALWAYS_INLINE void deque<T>::pop_front() noexcept {
    m_head->data<T>(m_head->begin_offset())->~T();
    m_head->set_begin_offset(m_head->begin_offset() + sizeof(T));
    if (m_head->begin_offset() == m_head->end_offset()) {
      m_head = detail::chunk::release_head(m_head);
      if (!m_head) {
        m_tail = nullptr;
      }
    }
  }

  template<typename T>
  J_NO_EXPLICIT J_ALWAYS_INLINE void deque<T>::pop_back() noexcept {
    m_tail->set_end_offset(m_tail->end_offset() - sizeof(T));
    m_tail->data<T>(m_tail->end_offset())->~T();
    if (m_tail->end_offset() == m_tail->begin_offset()) {
      m_tail = detail::chunk::release_tail(m_tail);
      if (!m_tail) {
        m_head = nullptr;
      }
    }
  }
}

#define J_DECLARE_EXTERN_DEQUE(...) extern template class j::deque<__VA_ARGS__>
