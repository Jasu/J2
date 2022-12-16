#pragma once

#include "hzd/type_traits.hpp"

namespace j::mem {
  template<typename T>
  class unique_ptr final {
  public:
    using nicely_copyable_tag_t = void;
    using zero_initializable_tag_t = void;

    J_ALWAYS_INLINE constexpr unique_ptr() noexcept = default;

    J_ALWAYS_INLINE constexpr unique_ptr(null_t) noexcept { }

    unique_ptr(const unique_ptr & rhs) = delete;
    unique_ptr & operator=(const unique_ptr & rhs) = delete;

    J_ALWAYS_INLINE explicit unique_ptr(T * t) noexcept : m_value(t) { }

    J_ALWAYS_INLINE unique_ptr(unique_ptr && rhs) noexcept : m_value(rhs.m_value)
    { rhs.m_value = nullptr; }

    template<typename U>
    J_ALWAYS_INLINE explicit unique_ptr(unique_ptr<U> && rhs) noexcept : m_value(rhs.m_value)
    { rhs.m_value = nullptr; }

    J_ALWAYS_INLINE unique_ptr & operator=(unique_ptr && rhs) noexcept
    { return operator=<T>(static_cast<unique_ptr &&>(rhs)); }

    template<typename U>
    unique_ptr & operator=(unique_ptr<U> && rhs) noexcept {
      if (J_LIKELY(this != &rhs)) {
        reset();
        m_value = rhs.m_value;
        rhs.m_value = nullptr;
      }
      return *this;
    }

    template<typename U>
    unique_ptr & operator=(U * rhs) noexcept {
      reset();
      return m_value = rhs, *this;
    }

    J_ALWAYS_INLINE T * release() noexcept {
      T * const result = m_value;
      return m_value = nullptr, result;
    }

    J_ALWAYS_INLINE_NONNULL T * operator->() const noexcept {
      J_ASSUME_NOT_NULL(m_value);
      return m_value;
    }

    J_ALWAYS_INLINE T & operator*() const noexcept { return *operator->(); }

    J_ALWAYS_INLINE bool operator!() const noexcept { return !m_value; }
    J_ALWAYS_INLINE explicit operator bool() const noexcept { return m_value; }

    J_ALWAYS_INLINE void reset() noexcept {
      if (m_value) {
        ::delete m_value;
        m_value = nullptr;
      }
    }

    J_ALWAYS_INLINE T * get() const noexcept { return m_value; }

    ~unique_ptr() { reset(); }
  private:
    T * m_value = nullptr;

    template<typename U> friend class unique_ptr;
  };

  template<typename T>
  class unique_ptr<T[]> final {
  public:
    using nicely_copyable_tag_t = void;
    using zero_initializable_tag_t = void;

    J_ALWAYS_INLINE constexpr unique_ptr() noexcept = default;

    unique_ptr(const unique_ptr & rhs) = delete;
    unique_ptr & operator=(const unique_ptr & rhs) = delete;

    J_ALWAYS_INLINE explicit unique_ptr(T * t) noexcept : m_value(t) { }

    template<typename U, typename = enable_if_t<is_same_v<const U, T>>>
    J_ALWAYS_INLINE explicit unique_ptr(U * t) noexcept : m_value(t) { }

    template<typename U, typename = enable_if_t<is_same_v<const U, T>>>
    J_ALWAYS_INLINE unique_ptr(unique_ptr<U []> && rhs) noexcept : m_value(rhs.m_value)
    { rhs.m_value = nullptr; }

    J_ALWAYS_INLINE unique_ptr(unique_ptr && rhs) noexcept : m_value(rhs.m_value)
    { rhs.m_value = nullptr; }

    unique_ptr & operator=(unique_ptr && rhs) noexcept {
      if (J_LIKELY(this != &rhs)) {
        reset();
        m_value = rhs.m_value;
        rhs.m_value = nullptr;
      }
      return *this;
    }

    template<typename U, typename = enable_if_t<is_same_v<const U, T>>>
    unique_ptr & operator=(unique_ptr<U []> && rhs) noexcept {
      reset();
      m_value = rhs.m_value;
      rhs.m_value = nullptr;
      return *this;
    }

    J_ALWAYS_INLINE T * release() noexcept {
      T * const result = m_value;
      return m_value = nullptr, result;
    }

    J_ALWAYS_INLINE bool operator!() const noexcept { return !m_value; }
    J_ALWAYS_INLINE explicit operator bool() const noexcept { return m_value; }

    J_ALWAYS_INLINE void reset() noexcept {
      if (m_value) {
        ::delete[] m_value;
        m_value = nullptr;
      }
    }

    J_INLINE_GETTER T * get() const noexcept { return m_value; }

    J_INLINE_GETTER T & operator[](sz_t i) const noexcept {
      J_ASSUME_NOT_NULL(m_value);
      return m_value[i];
    }

    J_ALWAYS_INLINE ~unique_ptr() { reset(); }
  private:
    T * m_value = nullptr;
    friend class unique_ptr<remove_const_t<T>[]>;
    friend class unique_ptr<const T[]>;
  };
}
