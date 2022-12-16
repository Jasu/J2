#pragma once

#include "strings/string_cmp.hpp"
#include "files/paths/path.hpp"

namespace j::files::inline paths {
  struct path_comparer final {
    J_INLINE_GETTER i64_t operator()(const path & lhs, const path & rhs) const noexcept {
      return strings::string_cmp(lhs.m_path, rhs.m_path);
    }
  };
}
