#pragma once

#include "exceptions/exceptions.hpp"
#include "type_id/type_id.hpp"
#include "strings/string.hpp"

namespace j::services {
  /// Exception thrown by subsystem initialization if the service is not
  /// supported.
  struct not_supported_exception : public virtual j::exceptions::exception {};

  /// Exception thrown during container definition errors.
  ///
  /// These exceptions often happen during static initialization, causing program
  /// termination.
  struct definition_error : public virtual j::exceptions::exception {};

  extern const tags::tag_definition<strings::string> err_service_name;
  extern const tags::tag_definition<strings::string> err_service_instance_name;
  extern const tags::tag_definition<type_id::type_id> err_service_type;
  extern const tags::tag_definition<strings::string> err_interface_name;
  extern const tags::tag_definition<type_id::type_id> err_interface_type;
  extern const tags::tag_definition<strings::string> err_setting_name;
}
