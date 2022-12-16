#pragma once

#include "services/container/injection_context.hpp"
#include "services/container/service_instance_state.hpp"
#include "services/detail/dependencies_t.hpp"
#include "mem/shared_ptr_fwd.hpp"

namespace j::services::arg {
  template<typename ArgumentType>
  struct named_setting_argument final {
    const char * m_name;
    inline constexpr static bool has_dependencies_v = true;

    using result_type = ArgumentType &;

    [[nodiscard]] J_NO_DEBUG inline bool has(void *, const services::injection_context * ic) const noexcept {
      return ic->service_instance_state->has_setting(typeid(ArgumentType), m_name);
    }

    [[nodiscard]] J_NO_DEBUG inline ArgumentType & get(void *, const services::injection_context * ic, services::detail::dependencies_t * deps) const {
      auto setting = ic->service_instance_state->get_setting(typeid(ArgumentType), m_name);
      ArgumentType & setting_ref = *reinterpret_cast<ArgumentType*>(setting.get());
      deps->emplace(static_cast<mem::shared_ptr<void> &&>(setting));
      return setting_ref;
    }
  };

  struct setting final {
    inline J_NO_DEBUG constexpr explicit setting(const char * J_NOT_NULL name) noexcept
      : m_name(name)
    {
    }

    template<typename Arg>
    [[nodiscard]] J_NO_DEBUG inline constexpr auto select(u32_t) const noexcept {
      return named_setting_argument<decay_t<Arg>>{m_name};
    }

    const char * m_name;
  };
}
