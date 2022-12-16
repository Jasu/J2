#pragma once

#include "lisp/assembly/linking/reloc.hpp"
#include "mem/buffer.hpp"

namespace j::lisp::assembly::inline buffers {
  struct buffer final {
    j::mem::buffer data;
    relocs_t relocs;

    J_INLINE_GETTER constexpr bool empty() const noexcept {
      return data.empty();
    }

    J_INLINE_GETTER constexpr bool operator!() const noexcept {
      return data.empty();
    }

    J_INLINE_GETTER constexpr explicit operator bool() const noexcept {
      return !data.empty();
    }

    J_INLINE_GETTER u32_t size() const noexcept {
      return data.size();
    }

    bool operator==(const buffer & rhs) const noexcept;
    void set_base(u32_t base) noexcept;
  };
}
