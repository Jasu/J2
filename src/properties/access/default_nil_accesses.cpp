#include "properties/access/default_accesses.hpp"
#include "properties/access/nil_access_registration.hpp"

namespace j::properties::access {
  namespace {
    nil_access_registration<std::nullptr_t> nullptr_access_registration("nullptr");
  }

  template<>
  const typed_access_definition * typed_access_definition_v<null_t> = &nil_access_registration<std::nullptr_t>::definition;
  template<>
  const wrappers::wrapper_definition * wrapper_definition_v<null_t> = &nil_access_registration<std::nullptr_t>::wrapper;
}
