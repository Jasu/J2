#include "functions/bound_function.hpp"

namespace j::functions {
  namespace detail {
    template<typename T>
    void deleter(void * ptr) noexcept {
      ::delete reinterpret_cast<T*>(ptr);
    }
  }

  template<typename T>
  class stored_function;

  template<typename Result, typename... Args>
  class stored_function<Result (Args ...)> : public bound_function<Result (Args ...)> {
  public:
    template<Result (*Fn)(Args...)>
    J_INLINE_GETTER constexpr static stored_function bind() noexcept;

    template<auto Fn, typename Class>
    J_INLINE_GETTER constexpr static stored_function bind(Class * o);

    template<auto Fn, typename Class>
    J_INLINE_GETTER constexpr static stored_function bind(Class && o);

    constexpr stored_function() noexcept = default;
    J_ALWAYS_INLINE explicit constexpr stored_function(Result (*fn)(Args...)) noexcept;

    J_ALWAYS_INLINE constexpr stored_function(stored_function && rhs) noexcept;
    J_ALWAYS_INLINE constexpr stored_function & operator=(stored_function && rhs) noexcept;

    ~stored_function();

    stored_function(const stored_function & rhs) = delete;
    stored_function & operator=(const stored_function & rhs) = delete;
  protected:
    J_ALWAYS_INLINE constexpr stored_function(void * object,
                                              Result (*fn)(void *, Args &&...),
                                              void (*deleter)(void*) noexcept) noexcept;
    void (*m_deleter)(void *) noexcept = nullptr;
  };

  template<typename Result, typename... Args>
  J_ALWAYS_INLINE constexpr stored_function<Result (Args...)>::stored_function(Result (*fn)(Args...)) noexcept
    : bound_function<Result (Args...)>(fn)
  {
  }

  template<typename Result, typename... Args>
  J_ALWAYS_INLINE constexpr stored_function<Result (Args...)>::stored_function(
    void * object,
    Result (*fn)(void *, Args &&...),
    void (*deleter)(void*) noexcept
  ) noexcept
    : bound_function<Result (Args...)>(object, fn),
      m_deleter(deleter)
  {
  }

  template<typename Result, typename... Args>
  template<Result (*Fn)(Args...)>
  J_INLINE_GETTER constexpr stored_function<Result (Args...)> stored_function<Result (Args...)>::bind() noexcept
  {
    return stored_function<Result (Args...)>{Fn};
  }

  template<typename Result, typename... Args>
  template<auto Fn, typename Class>
  J_INLINE_GETTER constexpr stored_function<Result (Args...)> stored_function<Result (Args...)>::bind(Class * o)
  {
    return stored_function<Result (Args...)>{ ::new Class(*o), &detail::object_bind_helper<Class, Fn, Result, Args...>::invoke, &detail::deleter<Class> };
  }

  template<typename Result, typename... Args>
  template<auto Fn, typename Class>
  J_INLINE_GETTER constexpr stored_function<Result (Args...)> stored_function<Result (Args...)>::bind(Class && o)
  {
    return stored_function<Result (Args...)>{ ::new Class(static_cast<Class &&>(o)), &detail::object_bind_helper<Class, Fn, Result, Args...>::invoke, &detail::deleter<Class> };
  }

  template<typename Result, typename... Args>
  J_ALWAYS_INLINE constexpr stored_function<Result (Args...)>::stored_function(stored_function && rhs) noexcept
    : bound_function<Result (Args...)>(static_cast<bound_function<Result (Args...)> &&>(rhs)),
      m_deleter(rhs.m_deleter)
  {
    rhs.m_deleter = nullptr;
    rhs.m_object = nullptr;
    rhs.m_member_fn = nullptr;
  }

  template<typename Result, typename... Args>
  J_ALWAYS_INLINE constexpr stored_function<Result (Args...)> & stored_function<Result (Args...)>::operator=(stored_function && rhs) noexcept {
    if (this != &rhs) {
      bound_function<Result (Args...)>::operator=(static_cast<bound_function<Result (Args...)> &&>(rhs));
      m_deleter = rhs.m_deleter;
      rhs.m_deleter = nullptr;
      rhs.m_object = nullptr;
      rhs.m_member_fn = nullptr;
    }
    return *this;
  }


  template<typename Result, typename... Args>
  stored_function<Result (Args...)>::~stored_function() {
    if (m_deleter) {
      m_deleter(bound_function<Result (Args...)>::m_object);
    }
  }
}
