#pragma once

#include "strings/string.hpp"
#include "strings/string_hash.hpp"
#include "containers/hash_map_fwd.hpp"

namespace j::strings {
  struct string_const_key final {
    using type = const_string_view;
    using arg_type = const_string_view;
    using getter_type = const_string_view;
  };
}
#define J_DECLARE_EXTERN_STRING_MAP(V) J_DECLARE_EXTERN_HASH_MAP(j::strings::string, V, HASH(j::strings::string_hash), CONST_KEY(j::strings::string_const_key))

J_DECLARE_EXTERN_STRING_MAP(j::strings::string);

namespace j::strings {
  template<typename T>
  using string_map = hash_map<strings::string, T, string_hash, j::equal_to<string>, string_const_key>;

  template<typename T>
  using string_map_iterator = hash_map_iterator<strings::string, T>;

  template<typename T>
  using const_string_map_iterator = hash_map_iterator<const strings::string, const T>;
}
