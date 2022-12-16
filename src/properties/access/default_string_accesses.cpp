#include "properties/access/default_accesses.hpp"
#include "properties/access/string_access_registration.hpp"
#include "strings/string.hpp"

namespace j::properties::access {
  namespace {
    struct J_TYPE_HIDDEN const_char_string_access {
      J_ALWAYS_INLINE static void deleter(void * str) noexcept {
        ::delete[] *static_cast<char**>(str);
        ::delete static_cast<char*>(str);
      }

      J_ALWAYS_INLINE static void destructor(void * str) noexcept {
        ::delete[] *static_cast<char**>(str);
      }

      J_INLINE_GETTER static strings::string as_j_string(const char * const & value) {
        return value;
      }

      J_INLINE_GETTER static strings::const_string_view as_j_string_view(const char * const & value) noexcept {
        return value;
      }

      J_INLINE_GETTER static sz_t get_size(const char * const & value) noexcept {
        return strlen(value);
      }

      J_INLINE_GETTER static const char * as_char_ptr(const char * const & value) noexcept {
        return value;
      }
    };

    struct J_TYPE_HIDDEN const_char_array_string_access {
      J_ALWAYS_INLINE static void deleter(void * str) noexcept {
        ::delete[] static_cast<char*>(str);
      }

      J_ALWAYS_INLINE static void destructor(void *) noexcept {
      }

      J_INLINE_GETTER static strings::string as_j_string(const char (& value)[]) {
        return value;
      }

      J_INLINE_GETTER static strings::const_string_view as_j_string_view(const char (& value)[]) noexcept {
        return value;
      }

      J_INLINE_GETTER static sz_t get_size(const char (& value) []) noexcept {
        return strlen(value);
      }

      J_INLINE_GETTER static const char * as_char_ptr(const char (&value)[]) noexcept {
        return value;
      }
    };

    struct J_TYPE_HIDDEN strings_string_access {
      J_INLINE_GETTER static strings::string as_j_string(const strings::string & value) {
        return value;
      }

      J_INLINE_GETTER static strings::const_string_view as_j_string_view(const strings::string & value) noexcept {
        return strings::const_string_view(value);
      }

      J_INLINE_GETTER static sz_t get_size(const strings::string & value) noexcept {
        return value.size();
      }

      J_INLINE_GETTER static const char * as_char_ptr(const strings::string & value) noexcept {
        return value.begin();
      }
    };

    struct J_TYPE_HIDDEN strings_string_view_access {
      J_INLINE_GETTER static strings::string as_j_string(const strings::const_string_view & value) {
        return value;
      }

      J_INLINE_GETTER static strings::const_string_view as_j_string_view(const strings::const_string_view & value) noexcept {
        return value;
      }

      J_INLINE_GETTER static sz_t get_size(const strings::const_string_view & value) noexcept {
        return value.size();
      }

      J_INLINE_GETTER static const char * as_char_ptr(const strings::const_string_view & value) noexcept {
        return value.begin();
      }
    };

    using const_char_registration_t J_NO_DEBUG_TYPE = string_access_registration<const char *, const_char_string_access>;
    const_char_registration_t const_char_registration{"const char *"};

    using const_char_array_registration_t J_NO_DEBUG_TYPE = string_access_registration<const char[], const_char_array_string_access>;
    const_char_array_registration_t const_char_array_registration("const char []");

    using strings_string_registration_t J_NO_DEBUG_TYPE = string_access_registration<strings::string, strings_string_access>;
    strings_string_registration_t strings_string_registration(
      "strings::string",
      convertible_from = [](const char * rhs) -> strings::string { return rhs; },
      convertible_from = [](const char (&rhs)[]) { return strings::string(&rhs[0]); },
      convertible_from = [](const strings::const_string_view & rhs) { return strings::string(rhs); }
    );

    using strings_string_view_registration_t J_NO_DEBUG_TYPE = string_access_registration<strings::const_string_view, strings_string_view_access>;
    strings_string_view_registration_t strings_string_view_registration(
      "strings::const_string_view",
      convertible_from = [](const char * rhs) noexcept -> strings::const_string_view { return rhs; },
      convertible_from = [](const char (&rhs)[]) noexcept { return strings::const_string_view(&rhs[0]); },
      convertible_from = [](const strings::string & rhs) noexcept { return strings::const_string_view(rhs); }
    );
  }

  template<> const wrappers::wrapper_definition * wrapper_definition_v<const char*> = &const_char_registration_t::wrapper;
  template<> const wrappers::wrapper_definition * wrapper_definition_v<const char[]> = &const_char_array_registration_t::wrapper;
  template<> const wrappers::wrapper_definition * wrapper_definition_v<strings::string> = &strings_string_registration_t::wrapper;
  template<> const wrappers::wrapper_definition * wrapper_definition_v<strings::const_string_view> = &strings_string_view_registration_t::wrapper;

  template<> const typed_access_definition * typed_access_definition_v<const char*> = &const_char_registration_t::definition;
  template<> const typed_access_definition * typed_access_definition_v<const char[]> = &const_char_array_registration_t::definition;
  template<> const typed_access_definition * typed_access_definition_v<strings::string> = &strings_string_registration_t::definition;
  template<> const typed_access_definition * typed_access_definition_v<strings::const_string_view> = &strings_string_view_registration_t::definition;

}
