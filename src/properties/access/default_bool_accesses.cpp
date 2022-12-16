#include "properties/access/default_accesses.hpp"
#include "properties/access/bool_access_registration.hpp"
#include "properties/assert.hpp"
#include "strings/string.hpp"

namespace j::properties::access {

  namespace {
    template<typename String>
    bool string_to_bool(const String & str) {
      if (str == "1" || str == "yes" || str == "true") {
        return true;
      }
      if (str == "0" || str == "no" || str == "false") {
        return false;
      }
      throw_invalid_conversion("Boolean could not be parsed from a string.");
    }

    strings::string bool_to_string(bool b) noexcept {
      return {b ? "true" : "false"};
    }

    bool_access_registration<bool> default_bool_access_registration(
      "bool",

      convertible_from = string_to_bool<strings::string>,
      convertible_from = string_to_bool<strings::const_string_view>,

      convertible_to = &bool_to_string
    );
  }

  template<>
  const wrappers::wrapper_definition * wrapper_definition_v<bool> = &bool_access_registration<bool>::wrapper;
  template<>
  const typed_access_definition * typed_access_definition_v<bool> = &bool_access_registration<bool>::definition;
}
