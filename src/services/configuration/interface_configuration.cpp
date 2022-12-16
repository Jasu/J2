#include "services/configuration/interface_configuration.hpp"
#include "properties/class_registration.hpp"
#include "properties/map_registration.hpp"
#include "containers/hash_map.hpp"
#include "containers/unsorted_string_map.hpp"

J_DEFINE_EXTERN_HASH_MAP(
  j::services::stored_service_reference,
  j::services::configuration::implementation_configuration,
  HASH(j::services::configuration::stored_service_reference_hash));

namespace j::services::configuration {
  interface_configuration::interface_configuration() noexcept
  {
  }

  interface_configuration::interface_configuration(bool is_per_service) noexcept
    : is_per_service(is_per_service) {
  }

  interface_configuration::interface_configuration(const interface_configuration & rhs)
    : default_implementation(rhs.default_implementation),
      implementations(rhs.implementations),
      is_per_service(rhs.is_per_service)
  {
  }

  interface_configuration::interface_configuration(interface_configuration && rhs) noexcept
    : default_implementation(static_cast<stored_service_reference &&>(rhs.default_implementation)),
      implementations(static_cast<implementations_t &&>(rhs.implementations)),
      is_per_service(rhs.is_per_service)
  {
  }

  interface_configuration & interface_configuration::operator=(const interface_configuration & rhs) {
    if (this != &rhs) {
      default_implementation = rhs.default_implementation;
      implementations = rhs.implementations;
      is_per_service = rhs.is_per_service;
    }
    return *this;
  }

  interface_configuration & interface_configuration::operator=(interface_configuration && rhs) noexcept {
    if (this != &rhs) {
      default_implementation = static_cast<stored_service_reference &&>(rhs.default_implementation);
      implementations = static_cast<implementations_t &&>(rhs.implementations);
      is_per_service = rhs.is_per_service;
    }
    return *this;
  }

  interface_configuration::~interface_configuration() {
  }

  // bool interface_configuration::operator==(const interface_configuration & rhs) const noexcept {
  //   return default_implementation == rhs.default_implementation
  //       && implementations == rhs.implementations
  //       && is_per_service == rhs.is_per_service;
  // }

  // bool interface_configuration::operator!=(const interface_configuration & rhs) const noexcept {
  //   return !operator==(rhs);
  // }

  // Reference the definition to hack global init order -.-''
  // TODO
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-variable-declarations"
  const auto & xx = properties::object_access_registration<interface_configuration>::definition;
#pragma clang diagnostic pop

  namespace {
    using namespace properties;

    // const map_access_registration<interface_configuration::implementations_t, properties::access::o rdered_map_access<interface_configuration::implementations_t>>
    //   map_reg("implementation_configuration");

    const object_access_registration<interface_configuration> reg{
      "interface_configuration",
      property = member<&interface_configuration::default_implementation>("default_implementation"),
      property = member<&interface_configuration::implementations>("implementations"),
      property = member<&interface_configuration::is_per_service>("is_per_service")
    };
  }
}
