#pragma once

#include "hzd/types.hpp"

namespace j::strings {
  template<typename> class basic_string_view;
  using const_string_view = basic_string_view<const char>;
  class string;
}

namespace j::properties::wrappers {
  class wrapper_definition;
}

namespace j::properties::access {
  class typed_access_definition;

  extern const typed_access_definition * const j_string_vector_access_definition;
  extern const wrappers::wrapper_definition * const j_string_vector_wrapper_definition;

  extern const typed_access_definition * const variant_list_access_definition;
  extern const wrappers::wrapper_definition * const variant_list_wrapper_definition;
  extern const typed_access_definition * const variant_map_access_definition;
  extern const wrappers::wrapper_definition * const variant_map_wrapper_definition;

  template<typename T> const wrappers::wrapper_definition * wrapper_definition_v;
  template<>           extern const wrappers::wrapper_definition * wrapper_definition_v<const char*>;
  template<>           extern const wrappers::wrapper_definition * wrapper_definition_v<const char[]>;
  template<>           extern const wrappers::wrapper_definition * wrapper_definition_v<strings::string>;
  template<>           extern const wrappers::wrapper_definition * wrapper_definition_v<strings::const_string_view>;
  template<>           extern const wrappers::wrapper_definition * wrapper_definition_v<null_t>;
  template<>           extern const wrappers::wrapper_definition * wrapper_definition_v<u8_t>;
  template<>           extern const wrappers::wrapper_definition * wrapper_definition_v<u16_t>;
  template<>           extern const wrappers::wrapper_definition * wrapper_definition_v<u32_t>;
  template<>           extern const wrappers::wrapper_definition * wrapper_definition_v<u64_t>;
  template<>           extern const wrappers::wrapper_definition * wrapper_definition_v<i8_t>;
  template<>           extern const wrappers::wrapper_definition * wrapper_definition_v<i16_t>;
  template<>           extern const wrappers::wrapper_definition * wrapper_definition_v<i32_t>;
  template<>           extern const wrappers::wrapper_definition * wrapper_definition_v<i64_t>;
  template<>           extern const wrappers::wrapper_definition * wrapper_definition_v<float>;
  template<>           extern const wrappers::wrapper_definition * wrapper_definition_v<double>;
  template<>           extern const wrappers::wrapper_definition * wrapper_definition_v<bool>;

  template<typename T> const typed_access_definition * typed_access_definition_v;
  template<>           extern const typed_access_definition * typed_access_definition_v<const char*>;
  template<>           extern const typed_access_definition * typed_access_definition_v<const char[]>;
  template<>           extern const typed_access_definition * typed_access_definition_v<strings::string>;
  template<>           extern const typed_access_definition * typed_access_definition_v<strings::const_string_view>;
  template<>           extern const typed_access_definition * typed_access_definition_v<null_t>;
  template<>           extern const typed_access_definition * typed_access_definition_v<u8_t>;
  template<>           extern const typed_access_definition * typed_access_definition_v<u16_t>;
  template<>           extern const typed_access_definition * typed_access_definition_v<u32_t>;
  template<>           extern const typed_access_definition * typed_access_definition_v<u64_t>;
  template<>           extern const typed_access_definition * typed_access_definition_v<i8_t>;
  template<>           extern const typed_access_definition * typed_access_definition_v<i16_t>;
  template<>           extern const typed_access_definition * typed_access_definition_v<i32_t>;
  template<>           extern const typed_access_definition * typed_access_definition_v<i64_t>;
  template<>           extern const typed_access_definition * typed_access_definition_v<float>;
  template<>           extern const typed_access_definition * typed_access_definition_v<double>;
  template<>           extern const typed_access_definition * typed_access_definition_v<bool>;

  template<typename T> inline constexpr bool has_default_access_v                       = false;
  template<>           inline constexpr bool has_default_access_v<const char*>          = true;
  template<>           inline constexpr bool has_default_access_v<const char[]>         = true;
  template<>           inline constexpr bool has_default_access_v<strings::string>      = true;
  template<>           inline constexpr bool has_default_access_v<strings::const_string_view> = true;
  template<>           inline constexpr bool has_default_access_v<null_t>               = true;
  template<>           inline constexpr bool has_default_access_v<u8_t>                 = true;
  template<>           inline constexpr bool has_default_access_v<u16_t>                = true;
  template<>           inline constexpr bool has_default_access_v<u32_t>                = true;
  template<>           inline constexpr bool has_default_access_v<u64_t>                = true;
  template<>           inline constexpr bool has_default_access_v<i8_t>                 = true;
  template<>           inline constexpr bool has_default_access_v<i16_t>                = true;
  template<>           inline constexpr bool has_default_access_v<i32_t>                = true;
  template<>           inline constexpr bool has_default_access_v<i64_t>                = true;
  template<>           inline constexpr bool has_default_access_v<float>                = true;
  template<>           inline constexpr bool has_default_access_v<double>               = true;
  template<>           inline constexpr bool has_default_access_v<bool>                 = true;
}
