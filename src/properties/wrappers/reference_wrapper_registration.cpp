#include "properties/wrappers/reference_wrapper_registration.hpp"

namespace j::properties::wrappers::detail {
  wrapper create_reference_wrapper(const wrapper_definition & def, access::typed_access & a) {
    return {def, a.as_void_star()};
  }
}
