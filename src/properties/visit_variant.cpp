#include "properties/visit_variant.hpp"
#include "exceptions/assert_lite.hpp"

namespace j::properties::detail {
  void throw_empty_visit() {
    J_THROW("Tried to visit an empty property.");
  }
  void throw_object_visit() {
    J_THROW("Tried to visit an object - variant in invalid state.");
  }
  void throw_enum_visit() {
    J_THROW("Tried to visit an enumeration - variant in invalid state.");
  }
  void throw_set_visit() {
    // TODO: Add set to variant
    J_THROW("Tried to visit a set - variant in invalid state.");
  }
}
