#pragma once

#include "exceptions/assert_lite.hpp"

namespace j::functions {
  namespace detail {
    template<typename FnPtr, typename Result, typename... Args>
    struct J_TYPE_HIDDEN dyn_fn_bind_helper;

    template<auto Fn, typename Result, typename... Args>
    struct J_TYPE_HIDDEN fn_bind_helper;

    template<class Class, auto Fn, typename Result, typename... Args>
    struct J_TYPE_HIDDEN object_bind_helper;

    template<class Class, class FnClass, typename FnResult, typename... FnArgs, FnResult (FnClass::*Fn) (FnArgs...), typename Result, typename... Args>
    struct J_TYPE_HIDDEN object_bind_helper<Class, Fn, Result, Args...> final {
      J_NO_DEBUG static Result invoke (void * J_NOT_NULL o, Args && ... args) {
        return (reinterpret_cast<Class*>(o)->*Fn)(static_cast<Args &&>(args)...);
      }
    };

    template<class Class, class FnClass, typename FnResult, typename... FnArgs, FnResult (FnClass::*Fn) (FnArgs...) const, typename Result, typename... Args>
    struct J_TYPE_HIDDEN object_bind_helper<Class, Fn, Result, Args...> final {
      J_NO_DEBUG static Result invoke (void * J_NOT_NULL o, Args && ... args) {
        if constexpr (is_void_v<Result>) {
          (reinterpret_cast<Class*>(o)->*Fn)(static_cast<Args &&>(args)...);
        } else {
          return (reinterpret_cast<Class*>(o)->*Fn)(static_cast<Args &&>(args)...);
        }
      }
    };

    template<typename FnResult, typename... FnArgs, FnResult (*Fn) (FnArgs...), typename Result, typename... Args>
    struct J_TYPE_HIDDEN fn_bind_helper<Fn, Result, Args...> final {
      J_NO_DEBUG static Result invoke (void*, Args && ... args) {
        if constexpr (is_void_v<Result>) {
          Fn(static_cast<Args &&>(args)...);
        } else {
          return Fn(static_cast<Args &&>(args)...);
        }
      }
    };

    template<typename FnResult, typename... FnArgs, typename Result, typename... Args>
    struct J_TYPE_HIDDEN dyn_fn_bind_helper<FnResult (*) (FnArgs...), Result, Args...> final {
      J_NO_DEBUG static Result invoke (void * J_NOT_NULL fn, Args && ... args) {
        if constexpr (is_void_v<Result>) {
          reinterpret_cast<FnResult (*)(FnArgs...)>(fn)(static_cast<Args &&>(args)...);
        } else {
          return reinterpret_cast<FnResult (*)(FnArgs...)>(fn)(static_cast<Args &&>(args)...);
        }
      }
    };
  }

  template<typename T>
  class bound_function;

  template<typename Result, typename... Args>
  class bound_function<Result (Args ...)> {
  protected:
    void * m_object = nullptr;
    Result (* m_member_fn) (void * J_NOT_NULL, Args &&...) = nullptr;

    J_ALWAYS_INLINE constexpr bound_function(
      void * object,
      Result (* J_NOT_NULL fn) (void * J_NOT_NULL, Args && ...)
    ) noexcept
      : m_object(object),
        m_member_fn(fn)
    {
    }

  public:
    J_ALWAYS_INLINE constexpr bound_function() noexcept = default;

    template<typename FnResult, typename... FnArgs>
    J_ALWAYS_INLINE explicit constexpr bound_function(FnResult(*fn)(FnArgs...)) noexcept
      : m_object(reinterpret_cast<void*>(fn)),
        m_member_fn(&detail::dyn_fn_bind_helper<FnResult(*)(FnArgs...), Result, Args...>::invoke)
    { }

    template<Result (*Fn)(Args...)>
    J_INLINE_GETTER constexpr static bound_function bind() noexcept
    { return bound_function{nullptr, &detail::fn_bind_helper<Fn, Result, Args...>::invoke }; }

    template<auto Fn, typename Class>
    J_INLINE_GETTER constexpr static bound_function bind(Class * J_NOT_NULL o) noexcept
    { return bound_function{o, &detail::object_bind_helper<Class, Fn, Result, Args...>::invoke}; }

    template<auto Fn, typename Class>
    J_INLINE_GETTER constexpr static bound_function bind(Class & o) noexcept
    { return bound_function{&o, &detail::object_bind_helper<Class, Fn, Result, Args...>::invoke}; }

    J_ALWAYS_INLINE constexpr bool operator!() const noexcept
    { return !m_member_fn; }

    J_ALWAYS_INLINE explicit constexpr operator bool() const noexcept
    { return m_member_fn != nullptr; }

    J_INLINE_GETTER constexpr bool empty() const noexcept
    { return !m_member_fn; }

    J_ALWAYS_INLINE_NO_DEBUG Result operator()(Args... args) const {
      J_ASSUME_NOT_NULL(m_member_fn);
      return m_member_fn(m_object, static_cast<Args &&>(args)...);
    }

    J_INLINE_GETTER constexpr bool operator==(const bound_function &) const noexcept = default;
  };
}
