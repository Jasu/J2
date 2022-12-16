#pragma once

#include "properties/path.hpp"
#include "strings/string_cmp.hpp"

namespace j::properties {
  inline i64_t path_cmp(const path & lhs, const path & rhs) noexcept {
    u32_t sz = lhs.size();
    if (sz != rhs.size()) {
      return sz - rhs.size();
    }
    while (sz--) {
      auto lhs_type = lhs[sz].type();
      {
        const i64_t type_diff = (int)lhs_type - (int)rhs[sz].type();
        if (type_diff) {
          return type_diff;
        }
      }
      switch (lhs_type) {
      case COMPONENT_TYPE::MAP_KEY:
      case COMPONENT_TYPE::PROPERTY_NAME: {
        const i64_t c = strings::string_cmp(lhs[sz].string_ref(), rhs[sz].string_ref());
        if (c != 0) {
          return c;
        }
        break;
      }
      case COMPONENT_TYPE::ARRAY_INDEX: {
        const i64_t c = lhs[sz].m_data - rhs[sz].m_data;
        if (c != 0) {
          return c;
        }
        break;
      }
      case COMPONENT_TYPE::EMPTY: break;
      }
    }
    return 0;
  }

  struct path_comparer final {
    J_INLINE_GETTER i64_t operator()(const path & lhs, const path & rhs) const noexcept {
      return path_cmp(lhs, rhs);
    }
  };
}
