#pragma once

#include "mem/shared_ptr.hpp"
#include "functions/bound_function.hpp"
#include "services/injected_calls/injected_call.hpp"

namespace j::services {
  class container;
  struct injection_context;
}

namespace j::services::injected_calls {
  template<typename Result, typename... PrefixArgs>
  class stored_injected_call {
  public:
    using fn_t J_NO_DEBUG_TYPE = functions::bound_function<Result (container *, const injection_context *, services::detail::dependencies_t *, PrefixArgs...)>;

    stored_injected_call() noexcept = default;

    template<bool Opt, typename C, typename ArgList>
    explicit stored_injected_call(const injected_call<Opt, C, ArgList> & fn);

    template<bool Opt, typename C, typename ArgList>
    explicit stored_injected_call(injected_call<Opt, C, ArgList> && fn);

    template<typename... Pref>
    J_ALWAYS_INLINE decltype(auto) operator()(container * c, const injection_context * ic, services::detail::dependencies_t * deps, Pref && ... prefix) const;

  private:
    mem::shared_ptr<void> m_store;
    fn_t m_fn;
  };

  template<typename Result, typename... PrefixArgs>
    template<bool Opt, typename C, typename ArgList>
  stored_injected_call<Result, PrefixArgs...>::stored_injected_call(const injected_call<Opt, C, ArgList> & fn)
    : m_store(mem::make_void_shared<injected_call<Opt, C, ArgList>>(fn)),
      m_fn(fn_t::template bind<&injected_call<Opt, C, ArgList>::template operator()<PrefixArgs...>>(
             reinterpret_cast<injected_call<Opt, C, ArgList> *>(m_store.get())))
  {
  }

  template<typename Result, typename... PrefixArgs>
    template<bool Opt, typename C, typename ArgList>
  stored_injected_call<Result, PrefixArgs...>::stored_injected_call(injected_call<Opt, C, ArgList> && fn)
    : m_store(mem::make_void_shared<injected_call<Opt, C, ArgList>>(static_cast<injected_call<Opt, C, ArgList> &&>(fn))),
      m_fn(fn_t::template bind<&injected_call<Opt, C, ArgList>::template operator()<PrefixArgs...>>(
             reinterpret_cast<injected_call<Opt, C, ArgList> *>(m_store.get())))
  {
  }

  template<typename Result, typename... PrefixArgs>
  template<typename... Pref>
  J_ALWAYS_INLINE decltype(auto) stored_injected_call<Result, PrefixArgs...>::operator()(container * c, const injection_context * ic, services::detail::dependencies_t * deps, Pref && ... prefix) const {
    return m_fn(c, ic, deps, static_cast<Pref &&>(prefix)...);
  }
}
