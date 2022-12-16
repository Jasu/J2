#pragma once

#include "exceptions/exceptions.hpp"

namespace j::properties {
  struct empty_property_access_exception : public exceptions::exception {};
  struct invalid_type_property_access_exception : public exceptions::exception {};
  struct invalid_conversion_exception : public exceptions::exception {};
}
