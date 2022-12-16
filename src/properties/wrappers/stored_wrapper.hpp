#pragma once

#include "properties/wrappers/wrapper.hpp"
#include "mem/shared_ptr_fwd.hpp"
#include <typeinfo>

namespace j::properties::wrappers {
  class stored_wrapper {
    /// Pointer to data in wrapper, to ensure that it is not freed.
    mem::shared_ptr<void> m_data; wrapper m_wrapper;
    static const wrapper_definition & from_registry(const std::type_info & type);

    template<typename T>
    struct J_TYPE_HIDDEN disable_if_self { using type = void; };

    template<>
    struct J_TYPE_HIDDEN disable_if_self<stored_wrapper &> { };

    template<>
    struct J_TYPE_HIDDEN disable_if_self<const stored_wrapper &> { };

  public:
    constexpr stored_wrapper() noexcept = default;

    template<typename T, typename = typename disable_if_self<T &>::type>
    J_ALWAYS_INLINE explicit stored_wrapper(T && t)
      : m_data(mem::make_void_shared<decltype(identity(t))>(static_cast<T &&>(t))),
        m_wrapper(from_registry(typeid(decltype(identity(t)))), m_data.get())
    {
    }


    template<typename Ptr>
    J_ALWAYS_INLINE stored_wrapper(const std::type_info & type, Ptr && ptr)
      : m_data(static_cast<Ptr &&>(ptr)),
        m_wrapper(from_registry(type), m_data.get())
      {
      }

    template<typename Wrapper, typename Ptr>
    J_ALWAYS_INLINE stored_wrapper(Wrapper && wrapper, Ptr && data)
      : m_data(static_cast<Ptr &&>(data)),
        m_wrapper(static_cast<Wrapper &&>(wrapper))
      {
  }

    ~stored_wrapper();

    auto & wrapper() noexcept {
      return m_wrapper;
    }

    const auto & wrapper() const noexcept {
      return m_wrapper;
    }

    const mem::shared_ptr<void> & data() const noexcept {
      return m_data;
    }
  };
}
