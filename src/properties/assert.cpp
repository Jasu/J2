#include "properties/assert.hpp"
#include "properties/exception.hpp"

namespace j::properties {
  [[noreturn]] void throw_invalid_conversion(const char * str) {
    J_THROW(invalid_conversion_exception() << message(str));
  }
}
