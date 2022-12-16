#pragma once

#include "properties/access/enum_access.hpp"
#include "properties/access/get_typed_access_definition.hpp"
#include "properties/access/typed_access_registration.hpp"
#include "properties/metadata.hpp"
#include "properties/wrappers/reference_wrapper_registration.hpp"
#include "strings/string_view.hpp"
#include "attributes/foreach.hpp"
#include "strings/formatters/enum_formatter.hpp"
#include "attributes/basic_operations.hpp"

namespace j::properties::access {
  namespace detail {
    class default_enum_access {};
  }

  template<typename Enum, typename Access = detail::default_enum_access>
  class J_TYPE_HIDDEN enum_access_registration {
  public:
    inline static enum_access_definition definition;
    inline static wrappers::reference_wrapper_definition<Enum, &definition, PROPERTY_TYPE::ENUM> wrapper;
  private:
    J_HIDDEN strings::formatters::enum_formatter<Enum> m_formatter;
    J_HIDDEN access::typed_access_registration m_registration;
    J_HIDDEN static u64_t to_uint(const void * v) noexcept {
      return (u64_t)*(const Enum *)(v);
    }
    J_HIDDEN static void from_uint(void * lhs, u64_t rhs) noexcept {
      *(Enum*)(lhs) = (Enum)rhs;
    }
  public:
    template<typename... Args, u32_t NumValues = ::j::attributes::count<Args...>(value)>
    J_HIDDEN explicit enum_access_registration(const char * name, Args && ... args)
      : m_formatter(NumValues)
    {
      if constexpr (NumValues != sizeof...(Args)) {
        convertible_from.maybe_apply([](const auto & ... convs) {
          auto & conversions = definition.conversions();
          (conversions.add_conversion_from(convs.from, convs.conversion), ...);
        }, static_cast<Args &&>(args)...);
      }
      initialize_typed_access<Enum, Access>(definition, PROPERTY_TYPE::ENUM, &wrapper, name);
      if constexpr (NumValues != sizeof...(Args)) {
        definition.metadata() = attributes::attribute_map(
          attributes::without(convertible_to, convertible_from, value),
          static_cast<Args &&>(args)...
        );
      }

      definition.initialize_enum_access(&to_uint, &from_uint);
      attributes::foreach_member(
        value,
        m_formatter,
        &strings::formatters::detail::enum_formatter_base::add_value,
        static_cast<Args &&>(args)...
      );

      m_registration.add_to_registry(typeid(Enum), &definition, &wrapper);
      if constexpr (NumValues != sizeof...(Args)) {
        convertible_to.maybe_apply(m_registration, static_cast<Args &&>(args)...);
      }
    }
  };
}
