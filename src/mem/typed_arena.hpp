#pragma once

#include "mem/page_pool.hpp"
#include "mem/page_range.hpp"
#include "containers/common.hpp"
#include "mem/common.hpp"

namespace j::mem {
  template<typename T>
  class typed_arena final {
  public:
    J_INLINE_GETTER u32_t size() const noexcept { return m_size; }

    T * begin() noexcept J_RETURNS_PAGE_ALIGNED
    { return (T*)m_page_range.address(); }
    J_INLINE_GETTER const T * begin() const noexcept J_RETURNS_PAGE_ALIGNED
    { return const_cast<typed_arena*>(this)->begin(); }

    [[nodiscard]] T * end() noexcept { return begin() + m_size; }
    [[nodiscard]] const T * end() const noexcept { return begin() + m_size; }

    [[nodiscard]] T & operator[](u32_t i) const noexcept {
      J_ASSERT(i < m_size);
      return (const_cast<typed_arena *>(this)->begin())[i];
    }

    [[nodiscard]] inline T & at(u32_t i) {
      if (J_UNLIKELY(i >= m_size)) {
        exceptions::throw_out_of_range(0, i);
      }
      return begin()[i];
    }

    [[nodiscard]] T & at(u32_t i) const {
      return const_cast<typed_arena*>(this)->at(i);
    }

    template<typename... Args>
    T & emplace(Args && ... args) {
      if (J_UNLIKELY(m_page_range.size() < (m_size + 1) * sizeof(T))) {
        if (!m_page_range) {
          m_page_range = vmem::page_pool::instance.allocate(1);
        } else {
          m_page_range.grow(vmem::allow_move, 1);
        }
        J_ASSERT(m_page_range.address());
      }
      T * res = ::new (begin() + m_size) T(static_cast<Args &&>(args)...);
      ++m_size;
      return *res;
    }

    template<typename Tag = return_allocation_tag>
    void clear(Tag = return_allocation) noexcept;

    ~typed_arena();
  private:
    vmem::page_range m_page_range;
    u32_t m_size = 0;
  };

  template<typename T>
  template<typename Tag>
  void typed_arena<T>::clear(Tag) noexcept {
    if (m_page_range) {
      if constexpr (!is_trivially_destructible_v<T>) {
        T * const ptr = begin();
        for (u32_t i = 0, sz = m_size; i < sz; ++i) {
          ptr[i].~T();
        }
      }
      m_size = 0U;
      if constexpr (is_same_v<Tag, return_allocation_tag>) {
        m_page_range.reset();
      }
    }
  }

  template<typename T>
  typed_arena<T>::~typed_arena() {
    clear();
  }
}
