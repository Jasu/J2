#pragma once

#include "files/paths/path.hpp"
#include "containers/hash_map_fwd.hpp"
#include "strings/string_hash.hpp"

namespace j::files::inline paths {

  struct path_hash final {
    J_A(NODISC,ND,FLATTEN) u32_t operator()(const path & p) const noexcept {
      return strings::hash_string(p.m_path.data(), p.m_path.size());
    }
  };

  template<typename V> using path_map = hash_map<path, V, path_hash>;
}

#define J_DECLARE_EXTERN_PATH_MAP(VALUE) J_DECLARE_EXTERN_HASH_MAP(j::files::path, VALUE, HASH(j::files::paths::path_hash))
