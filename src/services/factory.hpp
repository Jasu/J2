#pragma once

#include "services/injected_calls/injected_call.hpp"

namespace j::services {
  template<typename... Args, typename... FuncArgs>
  J_INLINE_GETTER static constexpr auto constructor(const FuncArgs & ... args) noexcept {
    return injected_calls::wire<Args...>(args...);
  }

  template<typename Result, typename... Args, typename... FuncArgs>
  J_INLINE_GETTER static constexpr auto factory(Result (*fn)(Args...), const FuncArgs & ... args) noexcept {
    return injected_calls::injected_call(
      injected_calls::required_tag_t::v,
      fn,
      injected_calls::wire<Args...>(args...));
  }
}
