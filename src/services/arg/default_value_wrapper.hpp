#pragma once

#include "services/detail/util.hpp"
#include "attributes/attribute_definition.hpp"
#include "attributes/basic_operations.hpp"

namespace j::services {
  class container;
  struct injection_context;
}

namespace j::services::arg {
  inline constexpr auto default_value = attributes::attribute_definition(
    attributes::tag = type<struct default_value_tag>);

  namespace detail {
    template<typename DefaultValue, typename Argument>
    struct default_value_wrapper {
      DefaultValue default_value;
      Argument argument;

      using result_type = typename Argument::result_type;

      constexpr static bool has_dependencies_v = Argument::has_dependencies_v;

      constexpr static bool has(services::container *, const services::injection_context *) noexcept {
        return true;
      }

      auto get(services::container * c, const services::injection_context * ic, services::detail::dependencies_t * deps) const {
        return argument.has(c, ic) ? argument.get(c, ic, deps) : default_value;
      }
    };

    template<typename DefaultValue, typename Argument>
    default_value_wrapper(DefaultValue, Argument) -> default_value_wrapper<DefaultValue, Argument>;

    template<typename DefaultValue, typename Selector>
    struct default_value_wrapper_selector {
      DefaultValue default_value;
      Selector selector;

      template<typename Argument>
      constexpr auto select(u32_t index) const noexcept(is_nothrow_constructible_v<DefaultValue, const DefaultValue &>) {
        return default_value_wrapper{
          default_value,
          selector.template select<Argument>(index),
        };
      }
    };

    template<typename DefaultValue, typename Selector>
    default_value_wrapper_selector(DefaultValue, Selector) -> default_value_wrapper_selector<DefaultValue, Selector>;
  }
}
