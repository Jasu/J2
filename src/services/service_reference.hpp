#pragma once

#include "strings/string_view.hpp"
#include "type_id/type_id.hpp"

namespace j::services {
  namespace arg {
    template<typename ArgumentType>
    class service_argument;
  }

  /// Reference to a service, either by name or service type, or both or none.
  ///
  /// Interface type is not included in the service reference, as service reference should be taken
  /// as configuration for instantiating an interface whose type is known at compile-time. This also
  /// allow specifying function arguments like `service(name = "logger")` without also having to
  /// specify the type.
  class service_reference {
  public:
    J_ALWAYS_INLINE constexpr service_reference() noexcept = default;

    J_ALWAYS_INLINE  explicit constexpr service_reference(type_id::type_id_getter service_type_id_getter) noexcept
      : m_service_type_id_getter(service_type_id_getter)
    {
    }

    J_ALWAYS_INLINE  explicit constexpr service_reference(type_id::type_id_getter service_type_id_getter,
                                const char * name) noexcept
      : m_service_type_id_getter(service_type_id_getter),
        m_service_instance_name(name ? name : "")
    {
    }

    J_ALWAYS_INLINE  explicit constexpr service_reference(const char * name) noexcept
      : m_service_instance_name(name ? name : "")
    {
    }

    type_id::type_id service_type_id() const noexcept;

    J_INLINE_GETTER constexpr type_id::type_id_getter service_type_id_getter() const noexcept {
      return m_service_type_id_getter;
    }

    J_INLINE_GETTER constexpr const strings::const_string_view & service_instance_name() const noexcept {
      return m_service_instance_name;
    }

    /// For using the service reference as a service argument.
    ///
    /// The definition resides in service_argument.hpp
    template<typename Arg>
    J_INLINE_GETTER constexpr arg::service_argument<Arg> select(u32_t) const noexcept;

    bool operator==(const service_reference & rhs) const noexcept;

    bool operator!=(const service_reference & rhs) const noexcept;

    bool operator<(const service_reference & rhs) const noexcept;

    bool operator>(const service_reference & rhs) const noexcept;

    bool operator<=(const service_reference & rhs) const noexcept;

    bool operator>=(const service_reference & rhs) const noexcept;

    bool empty() const noexcept;

    template<typename Type, typename String>
    J_INLINE_GETTER bool matches(const Type & type, const String & service_instance_name) const noexcept {
      if (!m_service_type_id_getter().empty() && type != m_service_type_id_getter()) {
        return false;
      }
      return m_service_instance_name.empty() || service_instance_name == m_service_instance_name;
    }
  private:
    /// Function pointer to get the type index of the service.
    type_id::type_id_getter m_service_type_id_getter = &type_id::get_empty_type_id;
    /// Name of the service instance.
    strings::const_string_view m_service_instance_name;
  };
}
