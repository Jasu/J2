#pragma once

#include "exceptions/assert_lite.hpp"
#include "mem/shared_ptr_fwd.hpp"

namespace j::rendering::vulkan::rendering {
  namespace detail {
    template<typename T, typename = bool>
    struct J_TYPE_HIDDEN has_release {
      J_NO_DEBUG static inline constexpr bool value = false;
    };

    template<typename T>
    struct J_TYPE_HIDDEN has_release<T, decltype(&T::release != nullptr)> {
      J_NO_DEBUG static inline constexpr bool value = true;
    };

    template<typename T>
    J_ALWAYS_INLINE static void reserved_resource_deleter(void * ptr) noexcept {
      J_ASSERT_NOT_NULL(ptr);
      auto t = reinterpret_cast<T*>(ptr);
      if constexpr (has_release<T>::value) {
        t->release();
      }
      ::delete t;
    }

    void reserved_resource_shared_deleter(void * ptr) noexcept;
  }

  class reserved_resource {
  public:
    using deleter_t = void (*)(void *) noexcept;
    void * m_value = nullptr;
    deleter_t m_deleter = nullptr;
  public:
    constexpr reserved_resource() noexcept = default;

    template<typename T>
    explicit reserved_resource(T * t) noexcept
      : m_value(t),
        m_deleter(&detail::reserved_resource_deleter<T>)
    {
      J_ASSERT_NOT_NULL(t);
    }

    template<typename T>
    explicit reserved_resource(const mem::shared_ptr<T> & t) noexcept
      : m_value(t.holder.control_block),
        m_deleter(&detail::reserved_resource_shared_deleter)
    {
      J_ASSERT_NOT_NULL(t);
      ++t.holder.control_block->num_references;
    }

    reserved_resource(const reserved_resource & rhs) = delete;
    reserved_resource & operator=(const reserved_resource & rhs) = delete;

    reserved_resource(reserved_resource && rhs) noexcept
      : m_value(rhs.m_value),
        m_deleter(rhs.m_deleter)
    {
      rhs.m_value = nullptr, rhs.m_deleter = nullptr;
    }

    reserved_resource & operator=(reserved_resource && rhs) noexcept;

    ~reserved_resource();

    J_INLINE_GETTER bool operator!() const noexcept {
      return !m_value;
    }

    J_INLINE_GETTER explicit operator bool() const noexcept {
      return m_value;
    }
  };

}
