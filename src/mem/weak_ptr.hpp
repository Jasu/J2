#pragma once

#include "mem/control_block.hpp"

namespace j::mem {
  namespace detail {
    template<typename T, typename>
    void set_weak_this(T * J_NOT_NULL t, shared_ptr_control_block * J_NOT_NULL cb) noexcept;
  }

  template<typename> class shared_ptr;

  template<typename T>
  class weak_ptr {
    template<typename U>
    friend class weak_ptr;

    T * m_target = nullptr;
    detail::shared_ptr_control_block * m_control_block = nullptr;

    void add_ref() noexcept {
      if (m_control_block) {
        ++m_control_block->num_weak_references;
      }
    }

  public:
    using nicely_copyable_tag_t J_NO_DEBUG_TYPE = void;
    using zero_initializable_tag_t J_NO_DEBUG_TYPE = void;

    J_ALWAYS_INLINE explicit weak_ptr(T * J_NOT_NULL target, detail::shared_ptr_control_block * J_NOT_NULL cb) noexcept
      : m_target(target),
        m_control_block(cb)
    { }
    J_ALWAYS_INLINE constexpr weak_ptr() noexcept = default;

    J_ALWAYS_INLINE constexpr weak_ptr(null_t) noexcept { }

    weak_ptr(const weak_ptr & rhs) noexcept
      : m_target(rhs.m_target),
        m_control_block(rhs.m_control_block)
    {
      add_ref();
    }

    J_ALWAYS_INLINE weak_ptr(weak_ptr && rhs) noexcept
      : m_target(rhs.m_target),
        m_control_block(rhs.m_control_block)
    {
      rhs.m_control_block = nullptr;
    }

    template<typename U>
    J_ALWAYS_INLINE weak_ptr(const shared_ptr<U> & rhs) noexcept
      : m_target(rhs.get()),
        m_control_block(rhs.holder.control_block)
    {
      add_ref();
    }

    weak_ptr & operator=(const weak_ptr & rhs) noexcept {
      if (J_LIKELY(this != &rhs)) {
        reset();
        m_target = rhs.m_target, m_control_block = rhs.m_control_block;
        add_ref();
      }
      return *this;
    }

    template<typename U>
    weak_ptr & operator=(const shared_ptr<U> & rhs) noexcept {
      reset();
      m_target = rhs.get();
      m_control_block = rhs.holder.control_block;
      add_ref();
      return *this;
    }

    weak_ptr & operator=(weak_ptr && rhs) noexcept {
      if (J_LIKELY(this != &rhs)) {
        reset();
        m_target = rhs.m_target, m_control_block = rhs.m_control_block;
        rhs.m_control_block = nullptr;
      }
      return *this;
    }

    void reset() noexcept {
      if (m_control_block) {
        if ((!--m_control_block->num_weak_references) && !m_control_block->num_references) {
          ::operator delete (static_cast<void*>(m_control_block));
        }
        m_control_block = nullptr;
      }
    }

    ~weak_ptr() { reset(); }

    J_INLINE_GETTER explicit operator bool() const noexcept
    { return m_control_block && m_control_block->num_references; }

    J_INLINE_GETTER bool operator!() const noexcept
    { return !m_control_block || !m_control_block->num_references; }

    [[nodiscard]] shared_ptr<T> lock() const noexcept {
      if (m_control_block && m_control_block->num_references) {
        ++m_control_block->num_references;
        return shared_ptr<T>{ m_target, m_control_block };
      } else {
        return shared_ptr<T>{};
      }
    }
  };
}

extern template class j::mem::weak_ptr<void>;
