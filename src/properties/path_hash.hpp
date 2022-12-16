#pragma once

#include "properties/path.hpp"
#include "strings/string_hash.hpp"

namespace j::properties {
  struct path_hash final {
    J_A(ND,FLATTEN,NI) u32_t operator()(const properties::path & p) const noexcept {
      u32_t hash = 3U;
      for (auto & part : p) {
        switch (part.type()) {
        case properties::COMPONENT_TYPE::MAP_KEY:
          hash -= 17;
          [[fallthrough]];
        case properties::COMPONENT_TYPE::PROPERTY_NAME:
          hash ^= strings::hash_string(part.string_ref());
          break;
        case properties::COMPONENT_TYPE::ARRAY_INDEX:
          hash = crc32(hash, part.as_array_index_noexcept());
          break;
        case properties::COMPONENT_TYPE::EMPTY:
          break;
        }
      }
      return hash;
    }
  };
}
