#pragma once

#include "services/injected_calls/stored_injected_call.hpp"
#include "containers/deque_fwd.hpp"
#include "containers/pair.hpp"
#include "cli/arguments.hpp"

J_DECLARE_EXTERN_DEQUE(j::pair<j::strings::const_string_view, j::cli::argument_definition>);

namespace j::cli::detail {
  struct callback final {
    services::injected_calls::stored_injected_call<void> invoke;
    deque<pair<strings::const_string_view, argument_definition>> arguments;

    J_ALWAYS_INLINE_NO_DEBUG static void add_argument_or_option(const void *) noexcept { }

    template<typename T, typename = typename T::cli_argument_tag>
    J_ALWAYS_INLINE_NO_DEBUG void add_argument_or_option(const T * J_NOT_NULL argument) {
      arguments.emplace_back(argument->spec, argument->definition);
    }

    template<typename T, typename = typename ::j::remove_ref_t<T>::injected_call_tag_t>
    J_ALWAYS_INLINE_NO_DEBUG callback(T && call) {
      call.get_arguments().as_tuple().apply([&](const auto & ... args) {
        (add_argument_or_option(&args), ...);
      });
      invoke = services::injected_calls::stored_injected_call<void>(static_cast<T &&>(call));
    }
  };
}
