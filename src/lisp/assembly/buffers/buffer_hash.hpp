#pragma once

#include "lisp/assembly/buffers/buffer.hpp"
#include "mem/memory_region_hash.hpp"
#include "lisp/assembly/linking/reloc_hash.hpp"

namespace j::lisp::assembly::inline buffers {
  struct buffer_hash final {
    J_A(ND,FLATTEN) u32_t operator()(const buffer & value) const noexcept {
      u32_t hash = region_hash(value.data);
      for (auto & r : value.relocs) {
        hash = crc32(hash, reloc_hash(r));
      }
      return hash;
    }

    [[no_unique_address]] reloc_hash reloc_hash;
    [[no_unique_address]] j::mem::memory_region_hash region_hash;
  };
}
