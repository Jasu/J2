#include "services/configuration/implementation_configuration.hpp"
#include "properties/class_registration.hpp"

namespace j::services::configuration {
  bool implementation_configuration::operator==(const implementation_configuration & rhs) const noexcept {
    return rhs.priority == priority;
  }
  bool implementation_configuration::operator!=(const implementation_configuration & rhs) const noexcept {
    return rhs.priority != priority;
  }
  bool implementation_configuration::operator<(const implementation_configuration & rhs) const noexcept {
    return rhs.priority < priority;
  }
  bool implementation_configuration::operator<=(const implementation_configuration & rhs) const noexcept {
    return rhs.priority <= priority;
  }
  bool implementation_configuration::operator>(const implementation_configuration & rhs) const noexcept {
    return rhs.priority > priority;
  }

  bool implementation_configuration::operator>=(const implementation_configuration & rhs) const noexcept {
    return rhs.priority >= priority;
  }

  namespace {
    properties::object_access_registration<implementation_configuration> reg(
      "implementation_configuration",

      properties::property = properties::member<&implementation_configuration::priority>("priority")
    );
  }
}
