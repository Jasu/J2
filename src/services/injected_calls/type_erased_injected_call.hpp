#pragma once

#include "services/injected_calls/injected_call.hpp"
#include "services/detail/util.hpp"

namespace j::services {
  class container;
  struct injection_context;
}

namespace j::services::injected_calls {
  namespace detail {
    struct dummy;
    using fn_ptr_t J_NO_DEBUG_TYPE = void (dummy::*)(void);

    template<typename WiredArgList>
    J_INTERNAL_LINKAGE void deleter(void * arglist) noexcept {
      ::delete reinterpret_cast<WiredArgList*>(arglist);
    }

    template<typename Service, typename ArgList, typename FnPtr>
    J_INTERNAL_LINKAGE void wrapper_fn(void * arg_list, fn_ptr_t fn_ptr, container * c, const injection_context * ic, services::detail::dependencies_t * deps, void * service) {
      reinterpret_cast<ArgList*>(arg_list)->invoke(reinterpret_cast<FnPtr>(fn_ptr), c, ic, deps, reinterpret_cast<Service*>(service));
    }
  }

  class type_erased_injected_call final {
  public:
    constexpr type_erased_injected_call() noexcept = default;

    template<typename WiredArgList, typename ResultT, typename Service, typename... Args>
    J_ALWAYS_INLINE type_erased_injected_call(ResultT (Service::* member_fn)(Args...), WiredArgList && args)
      : m_wired_argument_list(::new WiredArgList(static_cast<WiredArgList &&>(args))),
        m_member_fn(reinterpret_cast<detail::fn_ptr_t>(member_fn)),
        m_wrapper_fn(&detail::wrapper_fn<Service, WiredArgList, ResultT (Service::*)(Args...)>),
        m_deleter(&detail::deleter<WiredArgList>)
    {
    }

    type_erased_injected_call(type_erased_injected_call && rhs) noexcept
      : m_wired_argument_list(rhs.m_wired_argument_list),
        m_member_fn(rhs.m_member_fn),
        m_wrapper_fn(rhs.m_wrapper_fn),
        m_deleter(rhs.m_deleter)
    {
      rhs.m_deleter = nullptr;
    }

    type_erased_injected_call & operator=(type_erased_injected_call && rhs) noexcept
    {
      if (J_LIKELY(this != &rhs)) {
        m_wired_argument_list = rhs.m_wired_argument_list, m_member_fn = rhs.m_member_fn,
        m_wrapper_fn = rhs.m_wrapper_fn, m_deleter = rhs.m_deleter;
        rhs.m_deleter = nullptr;
      }
      return *this;
    }

    void reset() noexcept {
      if (m_deleter) {
        m_deleter(m_wired_argument_list);
        m_deleter = nullptr, m_wrapper_fn = nullptr;
      }
    }

    ~type_erased_injected_call() {
      reset();
    }

    void operator()(container * c, const injection_context * ic, services::detail::dependencies_t * deps, void * service) const {
      m_wrapper_fn(m_wired_argument_list, m_member_fn, c, ic, deps, service);
    }
  private:
    void * m_wired_argument_list;
    detail::fn_ptr_t m_member_fn;
    void (*m_wrapper_fn)(void *, detail::fn_ptr_t, container *, const injection_context *, services::detail::dependencies_t *, void *);
    void (*m_deleter)(void*) noexcept;
  };

  template<bool HasDeps>
  class type_erased_injected_call_wrapper final {
  public:
    inline constexpr static bool has_dependencies_v = HasDeps;

    template<typename Fn, typename ArgList>
    J_ALWAYS_INLINE explicit type_erased_injected_call_wrapper(Fn fn, ArgList && arg_list) noexcept
      : m_call(fn, static_cast<ArgList &&>(arg_list)) { }

    J_INLINE_GETTER operator type_erased_injected_call &&() noexcept {
      return static_cast<type_erased_injected_call &&>(m_call);
    }
  private:
    type_erased_injected_call m_call;
  };

  template<typename Fn, typename ArgList>
  explicit type_erased_injected_call_wrapper(Fn, ArgList) -> type_erased_injected_call_wrapper<ArgList::has_dependencies_v>;
}
