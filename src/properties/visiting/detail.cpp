#include "properties/visiting/detail.hpp"
#include "properties/exception.hpp"

namespace j::properties::visiting::detail {
  [[noreturn]] void throw_empty_visit() {
    J_THROW(empty_property_access_exception() << message("Tried to visit an empty property."));
  }
}
