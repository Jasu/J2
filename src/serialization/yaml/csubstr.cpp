/// \file Registers c4::csubstr as a string type.
///
/// c4::csubstr is the map key type in rapidyaml, so register it as a string
/// access.

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wextra-semi-stmt"
#include <c4/substr.hpp>
#pragma clang diagnostic pop

#include "serialization/yaml/csubstr.hpp"

#include "properties/access/string_access_registration.hpp"

#include "strings/string.hpp"

namespace j::serialization::yaml {
  namespace {
    struct J_HIDDEN csubstr_access {
      J_ALWAYS_INLINE static strings::string as_j_string(const c4::csubstr & value) {
        return strings::string(value.data(), value.size());
      }

      J_ALWAYS_INLINE static strings::const_string_view as_j_string_view(const c4::csubstr & value) noexcept {
        return strings::const_string_view(value.data(), value.size());
      }

      J_ALWAYS_INLINE static sz_t get_size(const c4::csubstr & value) noexcept {
        return value.size();
      }

      J_ALWAYS_INLINE static const char * as_char_ptr(const c4::csubstr & value) noexcept {
        return value.data();
      }
    };

    J_ALWAYS_INLINE c4::csubstr from_c_string(const char * cstr) noexcept {
      return c4::csubstr(cstr, strlen(cstr));
    }

    J_ALWAYS_INLINE c4::csubstr from_j_string(const strings::string & str) noexcept {
      return c4::csubstr(str.data(), str.size());
    }

    J_ALWAYS_INLINE c4::csubstr from_j_string_view(const strings::const_string_view & str) noexcept {
      return c4::csubstr(str.data(), str.size());
    }

    using csubstr_reg_t = J_NO_DEBUG_TYPE properties::access::string_access_registration<c4::csubstr, csubstr_access>;
    csubstr_reg_t reg(
      "c4::csubstr",
      properties::convertible_from = from_c_string,
      properties::convertible_from = from_j_string,
      properties::convertible_from = from_j_string_view);
  }

  const properties::access::typed_access_definition * const csubstr_access_definition = &csubstr_reg_t::definition;
}
