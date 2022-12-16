#pragma once

#include "exceptions/assert_lite.hpp"
#include "hzd/mem.hpp"

namespace j::detail {
  struct ptr_chunk final {
    u32_t size = 0U;
    u32_t capacity = 0U;

    J_INLINE_GETTER_NONNULL iptr_t * begin() const noexcept {
      return reinterpret_cast<iptr_t *>(const_cast<ptr_chunk*>(this + 1));
    }

    J_INLINE_GETTER_NONNULL iptr_t * end() const noexcept {
      return reinterpret_cast<iptr_t *>(const_cast<ptr_chunk*>(this + 1)) + size;
    }
  };

  class ptr_container_impl {
  public:
    constexpr static inline u32_t npos = U32_MAX;

    J_ALWAYS_INLINE constexpr ptr_container_impl() noexcept = default;

    constexpr ptr_container_impl(const ptr_container_impl & rhs)
      : m_ptr(rhs.m_ptr)
      {
        if (m_ptr < 0) {
          dup();
        }
      }

    ptr_container_impl & operator=(const ptr_container_impl & rhs) noexcept {
      if (J_LIKELY(&rhs != this)) {
        clear();
        m_ptr = rhs.m_ptr;
        if (m_ptr < 0) {
          dup();
        }
      }
      return *this;
    }

    J_ALWAYS_INLINE constexpr ptr_container_impl(ptr_container_impl && rhs) noexcept
      : m_ptr(rhs.m_ptr) {
      rhs.m_ptr = 0;
    }

    ptr_container_impl & operator=(ptr_container_impl && rhs) noexcept {
      if (J_LIKELY(&rhs != this)) {
        clear();
        m_ptr = rhs.m_ptr;
        rhs.m_ptr = 0;
      }
      return *this;
    }

    J_INLINE_GETTER bool empty() const noexcept {
      return !m_ptr;
    }
    J_INLINE_GETTER explicit operator bool() const noexcept {
      return m_ptr;
    }
    J_INLINE_GETTER bool operator!() const noexcept {
      return !m_ptr;
    }

    J_A(AI,ND) inline u32_t size() const noexcept {
      return m_ptr ? ((m_ptr < 0) ? reinterpret_cast<const ptr_chunk*>(-m_ptr)->size : 1) : 0;
    }

    void clear() noexcept {
      if (m_ptr < 0) {
        j::free(reinterpret_cast<void*>(-m_ptr));
      }
      m_ptr = 0;
    }

    J_ALWAYS_INLINE ~ptr_container_impl() {
      clear();
    }

  protected:
    void dup();

    bool iptr_contains(const iptr_t ptr) const noexcept {
      if (!m_ptr) {
        return false;
      }
      if (m_ptr > 0) {
        return m_ptr == ptr;
      }
      return do_contains(ptr);
    }

    void iptr_remove(iptr_t ptr) noexcept;

    bool do_contains(iptr_t ptr) const noexcept;
    u32_t do_index_of(iptr_t ptr) const noexcept;

    u32_t iptr_index_of(const iptr_t ptr) const noexcept {
      if (!m_ptr) {
        return npos;
      }
      if (m_ptr > 0) {
        return m_ptr == ptr ? 0 : npos;
      }
      return do_index_of(ptr);
    }

    J_INLINE_GETTER_NONNULL iptr_t * iptr_begin() const noexcept {
      return (m_ptr < 0)
        ? reinterpret_cast<ptr_chunk *>(-m_ptr)->begin()
        : const_cast<iptr_t*>(&m_ptr);
    }

    [[nodiscard]] iptr_t & iptr_at(u32_t i) const noexcept {
      if (m_ptr < 0) {
        auto chunk = reinterpret_cast<ptr_chunk *>(-m_ptr);
        J_ASSERT(chunk->size > i);
        return chunk->begin()[i];
      } else {
        J_ASSERT(m_ptr && !i);
        return const_cast<iptr_t &>(m_ptr);
      }
    }

    J_INLINE_GETTER_NONNULL iptr_t * iptr_end() const noexcept {
      return (m_ptr < 0)
        ? reinterpret_cast<ptr_chunk *>(-m_ptr)->end()
        : (const_cast<iptr_t*>(&m_ptr) + (m_ptr ? 1 : 0));
    }

    iptr_t m_ptr = 0ULL;
  };

  void ptr_set_add(iptr_t * J_NOT_NULL data, iptr_t ptr);
  // void ptr_set_merge(iptr_t * J_NOT_NULL data, iptr_t* const * begin, iptr_t* const * end);
  void ptr_array_push_back(iptr_t * J_NOT_NULL data, iptr_t ptr);
}

namespace j::inline containers {
  template<typename Target> class ptr_set final : public detail::ptr_container_impl {
  public:
    J_INLINE_GETTER_NONNULL Target * begin() noexcept {
      return reinterpret_cast<Target*>(iptr_begin());
    }

    J_INLINE_GETTER_NONNULL Target * end() noexcept {
      return reinterpret_cast<Target*>(iptr_end());
    }

    J_INLINE_GETTER_NONNULL const Target * begin() const noexcept {
      return reinterpret_cast<const Target*>(iptr_begin());
    }

    J_INLINE_GETTER_NONNULL const Target * end() const noexcept {
      return reinterpret_cast<const Target*>(iptr_end());
    }

    void add(Target ptr) {
      if (!m_ptr) {
        m_ptr = reinterpret_cast<iptr_t>(ptr);
        return;
      } else if (m_ptr != reinterpret_cast<iptr_t>(ptr)) {
        detail::ptr_set_add(&m_ptr, reinterpret_cast<iptr_t>(ptr));
      }
    }

    J_INLINE_GETTER Target & operator[](u32_t i) noexcept {
      return reinterpret_cast<Target &>(iptr_at(i));
    }

    J_INLINE_GETTER Target operator[](u32_t i) const noexcept {
      return reinterpret_cast<const Target &>(iptr_at(i));
    }

    // void add(const ptr_set & other) {
    //   if (!m_ptr) {
    //     *this = other;
    //     return;
    //   } else {
    //     detail::ptr_set_merge(&m_ptr, reinterpret_cast<iptr_t* const *>(other.begin()), reinterpret_t<iptr_t* const *>(other.end()));
    //   }
    // }

    void remove(Target ptr) {
      iptr_remove(reinterpret_cast<iptr_t>(ptr));
    }

    bool contains(const Target ptr) const noexcept {
      return iptr_contains(reinterpret_cast<iptr_t>(ptr));
    }

    u32_t index_of(const Target ptr) const noexcept {
      return iptr_index_of(reinterpret_cast<iptr_t>(ptr));
    }
  };

  template<typename Target> class ptr_array final : public detail::ptr_container_impl {
  public:
    J_INLINE_GETTER_NONNULL Target * begin() noexcept {
      return reinterpret_cast<Target*>(iptr_begin());
    }

    J_INLINE_GETTER_NONNULL Target * end() noexcept {
      return reinterpret_cast<Target*>(iptr_end());
    }

    J_INLINE_GETTER_NONNULL const Target * begin() const noexcept {
      return reinterpret_cast<const Target*>(iptr_begin());
    }

    J_INLINE_GETTER_NONNULL const Target * end() const noexcept {
      return reinterpret_cast<const Target*>(iptr_end());
    }

    [[nodiscard]] Target & operator[](u32_t i) noexcept {
      return reinterpret_cast<Target &>(iptr_at(i));
    }

    [[nodiscard]] Target operator[](u32_t i) const noexcept {
      return reinterpret_cast<const Target &>(iptr_at(i));
    }

    void remove(Target ptr) {
      iptr_remove(reinterpret_cast<iptr_t>(ptr));
    }

    void push_back(Target ptr) {
      if (!m_ptr) {
        m_ptr = reinterpret_cast<iptr_t>(ptr);
        return;
      } else if (m_ptr != reinterpret_cast<iptr_t>(ptr)) {
        detail::ptr_array_push_back(&m_ptr, reinterpret_cast<iptr_t>(ptr));
      }
    }

    bool contains(Target ptr) const noexcept {
      return iptr_contains(reinterpret_cast<iptr_t>(ptr));
    }
    u32_t index_of(Target ptr) const noexcept {
      return iptr_index_of(reinterpret_cast<iptr_t>(ptr));
    }
  };
}
