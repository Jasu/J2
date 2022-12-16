#pragma once

#include "hzd/concepts.hpp"
#include "exceptions/assert_lite.hpp"
#include "containers/common.hpp"
#include "hzd/mem.hpp"

namespace j::detail {
  struct resizeable_byte_region {
    J_BOILERPLATE(resizeable_byte_region, CTOR_NE_ND, COPY_DEL)

    J_A(AI,ND) inline resizeable_byte_region(resizeable_byte_region && rhs) noexcept
      : ptr(rhs.ptr)
    { rhs.ptr = nullptr; }

    inline resizeable_byte_region & operator=(resizeable_byte_region && rhs) noexcept {
      if (J_LIKELY(this != &rhs)) {
        if (ptr) {
          clear();
        }
        ptr = rhs.ptr;
        rhs.ptr = nullptr;
      }
      return *this;
    }

    void swap(resizeable_byte_region & rhs) noexcept {
      void * rptr = rhs.ptr;
      rhs.ptr = ptr;
      ptr = rptr;
    }

    J_A(ND) inline explicit resizeable_byte_region(u32_t size_bytes) noexcept
      : ptr(size_bytes ? ::j::allocate(size_bytes) : nullptr)
    { }


    J_A(ND) inline resizeable_byte_region(const resizeable_byte_region & other, u32_t size_bytes) noexcept
      : ptr(size_bytes ? ::j::allocate(size_bytes) : nullptr)
    {
      if (size_bytes) {
        ::j::memcpy(ptr, other.ptr, size_bytes);
      }
    }

    resizeable_byte_region(const resizeable_byte_region & other, u32_t size_bytes, u32_t capacity_bytes);

    J_A(ND) inline resizeable_byte_region(const void * J_NOT_NULL from, u32_t size_bytes) noexcept
      : ptr(size_bytes ? ::j::allocate(size_bytes) : nullptr)
    {
      if (size_bytes) {
        ::j::memcpy(ptr, from, size_bytes);
      }
    }


    resizeable_byte_region(const void * J_NOT_NULL from, u32_t size_bytes, u32_t capacity_bytes);

    J_A(AI,HIDDEN,ND,FLATTEN) inline void grow(u32_t size) noexcept {
      ptr = ::j::reallocate(ptr, size);
    }

    J_A(AI,FLATTEN) inline void move(void * J_NOT_NULL to, void * J_NOT_NULL from, u32_t size_bytes) noexcept {
      ::j::memmove(to, from, size_bytes);
    }

    void zero(void * J_NOT_NULL begin, u32_t size_bytes) noexcept;

    J_A(AI,FLATTEN) inline void clear() noexcept {
      ::j::free(ptr);
      ptr = nullptr;
    }

    inline ~resizeable_byte_region() {
      clear();
    }

    void * ptr = nullptr;
  };

  template<typename T>
  struct resizeable_region_base {
    using nicely_copyable_tag_t = void;
    using zero_initializable_tag_t = void;
    using iterator = T *;
    using const_iterator = const T *;
    using value_type = T;

    J_A(AI,NE,HIDDEN,ND) inline resizeable_region_base() noexcept = default;

    J_A(NE,AI,HIDDEN,ND) inline resizeable_region_base(resizeable_region_base && rhs) noexcept
      : m_region(static_cast<resizeable_byte_region &&>(rhs.m_region)),
        m_size(rhs.m_size),
        m_capacity(rhs.m_capacity)
    {
      rhs.m_size = 0U;
      rhs.m_capacity = 0U;
    }

    J_A(NE,AI,HIDDEN,ND) explicit resizeable_region_base(u32_t size)
      : m_region(size * sizeof(T)),
        m_capacity(size)
    {
    }

    J_INLINE_GETTER J_NO_EXPLICIT T * begin() noexcept {
      return reinterpret_cast<T*>(m_region.ptr);
    }

    J_INLINE_GETTER J_NO_EXPLICIT T * end() noexcept {
      return reinterpret_cast<T*>(m_region.ptr) + m_size;
    }

    J_INLINE_GETTER J_NO_EXPLICIT const T * begin() const noexcept {
      return reinterpret_cast<const T*>(m_region.ptr);
    }

    J_INLINE_GETTER J_NO_EXPLICIT const T * end() const noexcept {
      return reinterpret_cast<const T*>(m_region.ptr) + m_size;
    }


    J_INLINE_GETTER J_NO_EXPLICIT T & operator[](u32_t i) noexcept {
      return reinterpret_cast<T*>(m_region.ptr)[i];
    }

    J_INLINE_GETTER J_NO_EXPLICIT const T & operator[](u32_t i) const noexcept {
      return reinterpret_cast<const T*>(m_region.ptr)[i];
    }


    J_INLINE_GETTER J_NO_EXPLICIT const T & front() const noexcept {
      return *reinterpret_cast<const T*>(m_region.ptr);
    }

    J_INLINE_GETTER J_NO_EXPLICIT T & front() noexcept {
      return *reinterpret_cast<T*>(m_region.ptr);
    }

    J_INLINE_GETTER J_NO_EXPLICIT T & back() noexcept {
      return reinterpret_cast<T*>(m_region.ptr)[m_size - 1U];
    }

    J_INLINE_GETTER J_NO_EXPLICIT const T & back() const noexcept {
      return reinterpret_cast<const T*>(m_region.ptr)[m_size - 1U];
    }

    T & at(u32_t i) {
      if (J_UNLIKELY(i >= m_size)) {
        exceptions::throw_out_of_range(0, i);
      }
      return reinterpret_cast<T*>(m_region.ptr)[i];
    }

    J_INLINE_GETTER J_NO_EXPLICIT const T & at(u32_t i) const {
      return const_cast<resizeable_region_base*>(this)->at(i);
    }

    J_INLINE_GETTER J_NO_EXPLICIT u32_t size() const noexcept {
      return m_size;
    }

    J_INLINE_GETTER J_NO_EXPLICIT bool empty() const noexcept {
      return !m_size;
    }

    J_INLINE_GETTER J_NO_EXPLICIT explicit operator bool() const noexcept {
      return m_size;
    }

    J_INLINE_GETTER J_NO_EXPLICIT bool operator!() const noexcept {
      return !m_size;
    }

    J_INLINE_GETTER J_NO_EXPLICIT u32_t capacity() const noexcept {
      return m_capacity;
    }
  protected:
    template<typename U>
    J_ALWAYS_INLINE J_NO_EXPLICIT resizeable_region_base(U && region, u32_t size, u32_t capacity) noexcept
      :  m_region(static_cast<U &&>(region)),
         m_size(size),
         m_capacity(capacity)
    {
    }

    J_A(AI,NE,ND,HIDDEN) inline void swap(resizeable_region_base & other) noexcept {
      m_region.swap(other.m_region);
      const u32_t sz = m_size, cap = m_capacity;
      m_size = other.m_size;
      m_capacity = other.m_capacity;
      other.m_size = sz;
      other.m_capacity = cap;
    }

    resizeable_byte_region m_region;

    u32_t m_size = 0U;
    u32_t m_capacity = 0U;
  };

  template<typename T>
  struct resizeable_region_inner : resizeable_region_base<T> {
    using resizeable_region_base<T>::resizeable_region_base;

    template<typename Tag, typename U>
    J_A(ND) explicit resizeable_region_inner(const Tag &, U * J_NOT_NULL from, u32_t size, u32_t capacity = 0U)
      : resizeable_region_base<T>(max(capacity, size) * sizeof(T), size, max(capacity, size))
    {
      for (auto & it : *this) {
        if constexpr (is_same_v<Tag, move_tag>) {
          ::new (&it) T(static_cast<U &&>(*from++));
        } else {
          static_assert(is_same_v<Tag, copy_tag>);
          ::new (&it) T(*from++);
        }
      }
    }

    template<u32_t N>
    J_A(ND,NE,AI,HIDDEN) inline resizeable_region_inner(copy_tag, const T (&from)[N])
      : resizeable_region_inner<T>(containers::copy, from, N)
    { }

    J_A(ND) resizeable_region_inner(resizeable_region_inner && rhs) noexcept = default;

    J_A(NE,AI,HIDDEN) inline resizeable_region_inner & operator=(resizeable_region_inner && rhs) noexcept
      {
        if (J_LIKELY(this != &rhs)) {
          clear();
          m_region = static_cast<resizeable_byte_region &&>(rhs.m_region);
          m_size = rhs.m_size;
          m_capacity = rhs.m_capacity;
          rhs.m_size = 0U;
          rhs.m_capacity = 0U;
        }
        return *this;
      }

    void clear(bool release = true) noexcept {
      for (auto & o : *this) {
        o.~T();
      }
      m_size = 0U;
      if (release) {
        m_region.clear();
        m_capacity = 0U;
      }
    }

    ~resizeable_region_inner() {
      clear();
    }

    template<typename Tag, typename U>
    J_A(AI,NE,ND,HIDDEN) explicit resizeable_region_inner(const Tag & t, U * J_NOT_NULL begin, U * J_NOT_NULL end)
      : resizeable_region_inner(t, begin, end - begin)
    { }


    void reserve(u32_t capacity) {
      if (capacity > m_capacity) {
        resizeable_region_inner<T>::operator=(resizeable_region_inner<T>(containers::move, reinterpret_cast<T*>(resizeable_region_base<T>::m_region.ptr), m_size, capacity));
      }
    }

    void resize(u32_t size) {
      if (m_size < size) {
        reserve(size);
        T * p = end();
        if constexpr (is_zero_initializable_v<T> || is_trivially_default_constructible_v<T>) {
          ::j::memzero(p, (size - m_size) * sizeof(T));
        } else {
          for (u32_t i = m_size; i < size; ++i) {
            ::new (p++) T();
          }
        }
      } else if (m_size > size) {
        T * p = end();
        for (u32_t i = size; i < m_size; ++i) {
          (--p)->~T();
        }
      }
      m_size = size;
    }

    J_RETURNS_NONNULL J_NO_EXPLICIT T * erase(const T * J_NOT_NULL it) noexcept {
      T * it_ = const_cast<T*>(it);
      --m_size;
      for (T * const e = end(); it_ != e; ++it_) {
        *it_ = static_cast<T &&>(it_[1]);
      }
      it_->~T();
      return const_cast<T*>(it);
    }

    J_A(NE,AI,HIDDEN,RNN) inline T * erase(u32_t i) noexcept {
      return erase(begin() + i);
    }


    void pop_back(u32_t i = 1U) noexcept {
      J_ASSERT(i <= m_size);
      resize(m_size - i);
    }

    J_A(NE,AI,HIDDEN) inline void pop_front(u32_t num = 1U) noexcept {
      J_ASSERT(num < m_size);
      auto b = begin(), e = end(), f = b + num;
      while (f != e) {
        *(b++) = static_cast<T &&>(*(f++));
      }
      while (b != e) {
        (b++)->~T();
      }
      m_size -= num;
    }


    template<typename... Args>
    J_RETURNS_NONNULL T * emplace(const T * J_NOT_NULL it, Args && ... args) {
      T inserted(static_cast<Args &&>(args)...);
      T * cur = const_cast<T*>(it);
      if (m_size == m_capacity) {
        u32_t index = cur - reinterpret_cast<T*>(resizeable_region_base<T>::m_region.ptr);
        reserve(m_capacity + clamp(4U, m_capacity, 8U));
        cur = reinterpret_cast<T*>(resizeable_region_base<T>::m_region.ptr) + index;
      }
      T * e = end();
      ++m_size;
      if (cur == e) {
        return ::new (cur) T(static_cast<T &&>(inserted));
      }

      ::new (e) T(static_cast<T &&>(e[-1]));
      --e;
      for (; e != cur; --e) {
        *e = static_cast<T &&>(e[-1]);
      }
      *cur = static_cast<T &&>(inserted);
      return cur;
    }

    using resizeable_region_base<T>::begin;
    using resizeable_region_base<T>::end;
  protected:
    using resizeable_region_base<T>::m_region;
    using resizeable_region_base<T>::m_size;
    using resizeable_region_base<T>::m_capacity;
  };

  template<TaggedNicelyCopyable T>
  struct resizeable_region_inner<T> : resizeable_region_base<T> {
    using resizeable_region_base<T>::resizeable_region_base;

    template<u32_t N>
    J_A(NE,ND,AI,HIDDEN) inline resizeable_region_inner(copy_tag, const T (&from)[N])
      : resizeable_region_inner<T>(containers::copy, from, N)
    { }

    J_A(ND) resizeable_region_inner(resizeable_region_inner && rhs) noexcept = default;

    resizeable_region_inner & operator=(resizeable_region_inner && rhs) noexcept
      {
        if (J_LIKELY(this != &rhs)) {
          clear();
          m_region = static_cast<resizeable_byte_region &&>(rhs.m_region);
          m_size = rhs.m_size;
          m_capacity = rhs.m_capacity;
          rhs.m_size = 0U;
          rhs.m_capacity = 0U;
        }
        return *this;
      }

    void clear(bool release = true) noexcept {
      for (auto & o : *this) {
        o.~T();
      }
      m_size = 0U;
      if (release) {
        m_region.clear();
        m_capacity = 0U;
      }
    }

    J_A(AI,NE,HIDDEN) inline ~resizeable_region_inner() {
      clear();
    }

    template<typename Tag, typename U>
    J_A(AI,ND,NE,HIDDEN) inline explicit resizeable_region_inner(const Tag &, U * J_NOT_NULL from, u32_t size, u32_t capacity = 0U)
      : resizeable_region_base<T>(max(capacity, size) * sizeof(T), size, max(capacity, size))
    {
      for (auto & it : *this) {
        if constexpr (is_same_v<Tag, move_tag>) {
          ::new (&it) T(static_cast<U &&>(*from++));
        } else {
          static_assert(is_same_v<Tag, copy_tag>);
          ::new (&it) T(*from++);
        }
      }
    }

    template<typename Tag, typename U>
    J_A(AI,ND,NE,HIDDEN) inline explicit resizeable_region_inner(const Tag & t, U * J_NOT_NULL begin, U * J_NOT_NULL end)
      : resizeable_region_inner(t, begin, end - begin)
    { }

    void reserve(u32_t capacity) {
      if (capacity > m_capacity) {
        m_region.grow(capacity * sizeof(T));
        m_capacity = capacity;
      }
    }

    J_RETURNS_NONNULL T * erase(const T * J_NOT_NULL it) noexcept {
      T * it_ = const_cast<T*>(it);
      J_ASSERT(it_ >= begin() && it_ < end());
      it_->~T();
      --m_size;
      if (u32_t sz = byte_offset(it_, end())) {
        m_region.move(it_, it_ + 1, sz);
      }
      return it_;
    }

    J_ALWAYS_INLINE J_NO_EXPLICIT J_RETURNS_NONNULL T * erase(u32_t i) noexcept {
      return erase(begin() + i);
    }

    void resize(u32_t size) {
      if (m_size < size) {
        reserve(size);
        T * p = end();
        if constexpr (is_zero_initializable_v<T> || is_trivially_default_constructible_v<T>) {
          ::j::memzero(p, (size - m_size) * sizeof(T));
        } else {
          for (u32_t i = m_size; i < size; ++i) {
            ::new (p++) T();
          }
        }
      } else if (m_size > size) {
        T * p = end();
        for (u32_t i = size; i < m_size; ++i) {
          (--p)->~T();
        }
      }
      m_size = size;
    }

    J_ALWAYS_INLINE J_NO_EXPLICIT void pop_back(u32_t i = 1U) noexcept {
      J_ASSERT(m_size <= m_capacity && i <= m_size);
      for (T * const e = end(), *it = e - i; it != e; ++it) {
        it->~T();
      }
      m_size -= i;
    }

    J_NO_EXPLICIT J_HIDDEN void pop_front(u32_t num = 1U) noexcept {
      J_ASSERT(m_size <= m_capacity && num <= m_size);
      T * b = begin();
      for (u32_t i = 0; i < num; ++i) {
        (b++)->~T();
      }
      m_region.move(begin(), b, byte_offset(b, end()));
      m_size -= num;
    }


    template<typename... Args>
    J_A(RNN,ND) T * emplace(const T * J_NOT_NULL it, Args && ... args) {
      T inserted(static_cast<Args &&>(args)...);
      T * cur = const_cast<T*>(it);
      if (m_size == m_capacity) {
        const u32_t index = it - reinterpret_cast<T*>(resizeable_region_base<T>::m_region.ptr);
        reserve(m_capacity + clamp(4U, m_capacity, 8U));
        cur = reinterpret_cast<T*>(resizeable_region_base<T>::m_region.ptr) + index;
      }
      T * const e = end();
      if (cur != e) {
        m_region.move(cur + 1, cur, byte_offset(cur, e));
      }
      ++m_size;
      return ::new (cur) T(static_cast<T &&>(inserted));
    }

    using resizeable_region_base<T>::begin;
    using resizeable_region_base<T>::end;
  protected:
    using resizeable_region_base<T>::m_region;
    using resizeable_region_base<T>::m_size;
    using resizeable_region_base<T>::m_capacity;
  };

  template<TriviallyCopyable T>
  struct resizeable_region_inner<T> : resizeable_region_base<T> {
    using resizeable_region_base<T>::resizeable_region_base;

    template<u32_t N>
    J_A(AI,ND,NE,HIDDEN) inline  resizeable_region_inner(copy_tag, const T (&from)[N])
      : resizeable_region_inner<T>(containers::copy, from, N)
    { }

    J_A(ND) inline resizeable_region_inner(resizeable_region_inner && rhs) noexcept = default;

    J_A(AI,ND,NE,HIDDEN) inline resizeable_region_inner & operator=(resizeable_region_inner && rhs) noexcept
    {
      if (J_LIKELY(this != &rhs)) {
        m_region = static_cast<resizeable_byte_region &&>(rhs.m_region);
        m_size = rhs.m_size;
        m_capacity = rhs.m_capacity;
        rhs.m_size = 0U;
        rhs.m_capacity = 0U;
      }
      return *this;
    }

    inline void clear(bool release = true) noexcept {
      m_size = 0U;
      if (release) {
        m_region.clear();
        m_capacity = 0U;
      }
    }

    J_A(AI,NE,HIDDEN) inline ~resizeable_region_inner() {
      m_region.clear();
    }

    template<typename Tag>
    J_A(AI,ND,NE,HIDDEN) inline explicit resizeable_region_inner(const Tag &, const T * J_NOT_NULL from, u32_t size, u32_t capacity = 0U)
      : resizeable_region_base<T>(resizeable_byte_region(from, size * sizeof(T), max(capacity, size) * sizeof(T)), size, max(capacity, size))
    { static_assert(is_same_v<Tag, copy_tag> || is_same_v<Tag, move_tag>); }

    template<typename Tag>
    J_A(AI,ND,NE,HIDDEN) inline explicit resizeable_region_inner(const Tag &, const T * J_NOT_NULL begin, const T * J_NOT_NULL end)
      : resizeable_region_inner(containers::move, begin, end - begin)
    { }

    void reserve(u32_t capacity) {
      if (capacity > m_capacity) {
        m_region.grow(capacity * sizeof(T));
        m_capacity = capacity;
      }
    }

    void resize(u32_t size) {
      if (m_size < size) {
        reserve(size);
        T * p = end();
        if constexpr (is_zero_initializable_v<T> || is_trivially_default_constructible_v<T>) {
          ::j::memzero(p, (size - m_size) * sizeof(T));
        } else {
          for (u32_t i = m_size; i < size; ++i) {
            ::new (p++) T();
          }
        }
      }
      m_size = size;
    }

    J_RETURNS_NONNULL T * erase(const T * J_NOT_NULL it) noexcept {
      T * it_ = const_cast<T*>(it);
      if (u32_t sz = byte_offset(it_, begin() + --m_size)) {
        m_region.move(it_, it_ + 1, sz);
      }
      return it_;
    }

    J_ALWAYS_INLINE J_NO_EXPLICIT J_RETURNS_NONNULL T * erase(u32_t i) noexcept {
      return erase(begin() + i);
    }

    J_ALWAYS_INLINE J_NO_EXPLICIT void pop_back(u32_t i = 1U) noexcept {
      J_ASSERT(i <= m_size);
      m_size -= i;
    }

    J_HIDDEN J_NO_EXPLICIT void pop_front(u32_t i = 1U) noexcept {
      J_ASSERT(i <= m_size);
      const auto b = begin();
      m_size -= i;
      m_region.move(b, b + i, m_size * sizeof(T));
    }


    template<typename... Args>
    J_A(RNN) T * emplace(const T * J_NOT_NULL it, Args && ... args) {
      T inserted(static_cast<Args &&>(args)...);
      T * cur = const_cast<T*>(it);
      if (m_size == m_capacity) {
        const u32_t index = it - reinterpret_cast<T*>(m_region.ptr);
        reserve(m_capacity + clamp(4U, m_capacity, 8U));
        cur = reinterpret_cast<T*>(m_region.ptr) + index;
      }
      T * const e = end();
      if (cur != e) {
        m_region.move(cur + 1, cur, byte_offset(cur, e));
      }
      ++m_size;
      return ::new (cur) T(static_cast<T &&>(inserted));
    }

    using resizeable_region_base<T>::begin;
    using resizeable_region_base<T>::end;
  protected:

    using resizeable_region_base<T>::m_region;
    using resizeable_region_base<T>::m_size;
    using resizeable_region_base<T>::m_capacity;
  };

  template<typename T>
  struct resizeable_region_inner_copyable : resizeable_region_inner<T> {
    using resizeable_region_inner<T>::resizeable_region_inner;
    J_A(AI,ND,NE,HIDDEN) inline resizeable_region_inner_copyable() noexcept = default;

    J_A(ND) resizeable_region_inner_copyable(const resizeable_region_inner_copyable & rhs)
      : resizeable_region_inner<T>(rhs.m_size * sizeof(T), rhs.m_size, rhs.m_size)
    {
      if (rhs.m_size) {
        copy_from(rhs.m_region);
      }
    }

    template<u32_t N>
    J_A(AI,ND,NE,HIDDEN) inline resizeable_region_inner_copyable(const T (&from)[N])
      : resizeable_region_inner<T>(containers::copy, from, N)
    { }

    J_A(AI,NE,ND,HIDDEN) inline resizeable_region_inner_copyable & operator=(const resizeable_region_inner_copyable & rhs) {
      if (J_LIKELY(this != &rhs)) {
        resizeable_region_inner<T>::clear();
        m_region = resizeable_byte_region(rhs.m_size * sizeof(T));
        m_size = rhs.m_size;
        if ((m_capacity = rhs.m_size)) {
          copy_from(rhs.m_region);
        }
      }
      return *this;
    }

  protected:
    void copy_from(const resizeable_byte_region & region) noexcept(is_nothrow_copy_constructible_v<T>) {
      auto from = reinterpret_cast<const T *>(region.ptr);
      for (auto & it : *this) {
        ::new (&it) T(*from++);
      }
    }

    using resizeable_region_inner<T>::m_size;
    using resizeable_region_inner<T>::m_capacity;
    using resizeable_region_inner<T>::m_region;
  };

  template<TriviallyCopyable T>
  struct resizeable_region_inner_copyable<T> : resizeable_region_inner<T> {
    using resizeable_region_inner<T>::resizeable_region_inner;
    J_A(AI,ND,NE,HIDDEN) inline resizeable_region_inner_copyable() noexcept = default;

    J_A(ND) inline resizeable_region_inner_copyable(const resizeable_region_inner_copyable & rhs)
      : resizeable_region_inner<T>(resizeable_byte_region(rhs.m_region, rhs.m_size * sizeof(T)), rhs.m_size, rhs.m_size)
    { }

    template<u32_t N>
    J_A(AI,ND,NE,HIDDEN) inline resizeable_region_inner_copyable(const T (&from)[N])
      : resizeable_region_inner<T>(resizeable_byte_region(from, N * sizeof(T)), N, N)
    { }

    J_A(AI,ND,NE,HIDDEN) inline resizeable_region_inner_copyable & operator=(const resizeable_region_inner_copyable & rhs) {
      if (J_LIKELY(this != &rhs)) {
        m_region = resizeable_byte_region(rhs.m_region, rhs.m_size * sizeof(T));
        m_size = rhs.m_size;
        m_capacity = rhs.m_size;
      }
      return *this;
    }

  protected:
    using resizeable_region_inner<T>::m_size;
    using resizeable_region_inner<T>::m_capacity;
    using resizeable_region_inner<T>::m_region;
  };


  template<typename T>
  struct resizeable_region : resizeable_region_inner<T> {
    using resizeable_region_inner<T>::resizeable_region_inner;
    J_A(AI,ND,NE,HIDDEN) inline resizeable_region() noexcept = default;

    template<typename... Args>
    T & emplace_back(Args && ... args) {
      T inserted(static_cast<Args &&>(args)...);
      if (m_size == m_capacity) {
        resizeable_region_inner<T>::reserve(m_capacity + clamp(4U, m_capacity, 8U));
      }
      return *::new (reinterpret_cast<T*>(resizeable_region_base<T>::m_region.ptr) + m_size++) T{static_cast<T &&>(inserted)};
    }

    template<typename... Args>
    J_ALWAYS_INLINE T & emplace_front(Args && ... args) {
      return *emplace(reinterpret_cast<T*>(resizeable_region_base<T>::m_region.ptr), static_cast<Args&&>(args)...);
    }

    template<typename V = T>
    J_A(RNN) T * insert(const T * J_NOT_NULL at, V && value) {
      return emplace(at, static_cast<V &&>(value));
    }

    template<typename V = T>
    T & push_back(V && value) {
      return emplace_back(static_cast<V &&>(value));
    }

    template<typename V = T>
    J_A(AI,HIDDEN) inline T & push_front(V && value) {
      return *emplace(reinterpret_cast<T*>(resizeable_region_base<T>::m_region.ptr), static_cast<V &&>(value));
    }

    using resizeable_region_inner<T>::emplace;
    using resizeable_region_inner<T>::begin;
    using resizeable_region_inner<T>::end;
  protected:
    using resizeable_region_inner<T>::m_capacity;
    using resizeable_region_inner<T>::m_size;
  };

  template<typename T>
  struct resizeable_region_copyable : resizeable_region_inner_copyable<T> {
    using resizeable_region_inner_copyable<T>::resizeable_region_inner_copyable;
    resizeable_region_copyable() noexcept = default;

    template<typename... Args>
    T & emplace_back(Args && ... args) {
      T inserted(static_cast<Args &&>(args)...);
      if (m_size == resizeable_region_inner_copyable<T>::m_capacity) {
        resizeable_region_inner<T>::reserve(m_capacity + clamp(4U, m_capacity, 8U));
      }
      return *::new (reinterpret_cast<T*>(resizeable_region_base<T>::m_region.ptr) + m_size++) T(static_cast<T &&>(inserted));
    }

    template<typename... Args>
    T & emplace_front(Args && ... args) {
      return *emplace(reinterpret_cast<T*>(resizeable_region_base<T>::m_region.ptr), static_cast<Args&&>(args)...);
    }

    template<typename V = T>
    J_A(RNN) T * insert(const T * J_NOT_NULL at, V && value) {
      return emplace(at, static_cast<V &&>(value));
    }

    template<typename V = T>
    T & push_back(V && value) {
      return emplace_back(static_cast<V &&>(value));
    }

    template<typename V = T>
    J_A(AI,HIDDEN,NE,HIDDEN) inline T & push_front(V && value) {
      return *emplace(reinterpret_cast<T*>(resizeable_region_base<T>::m_region.ptr), static_cast<V &&>(value));
    }

    using resizeable_region_inner_copyable<T>::emplace;
    using resizeable_region_inner_copyable<T>::begin;
    using resizeable_region_inner_copyable<T>::end;
  protected:
    using resizeable_region_inner_copyable<T>::m_capacity;
    using resizeable_region_inner_copyable<T>::m_size;
  };
}
