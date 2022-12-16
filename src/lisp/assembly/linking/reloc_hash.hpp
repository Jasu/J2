#pragma once

#include "hzd/crc32.hpp"
#include "lisp/assembly/linking/reloc.hpp"

namespace j::lisp::assembly::inline linking {
  struct reloc_source_hash final {
    J_INLINE_GETTER u32_t operator()(const reloc_source & value) const noexcept {
      return crc32(12, (const char*)&value, sizeof(reloc_source));
    }
  };

  struct reloc_hash final {
    J_INLINE_GETTER u32_t operator()(const reloc & value) const noexcept {
      return crc32(17, (const char*)&value, sizeof(reloc));
    }
  };
}
