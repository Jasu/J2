#pragma once

#include "lisp/common/metadata.hpp"
#include "lisp/common/id.hpp"
#include "strings/string_view.hpp"

namespace j::lisp::assembly::inline functions {
  struct alignas(u64_t) addr_info final {
    u32_t offset = 0U;
    bool has_metadata = false;
    u8_t label_size = 0U;

    metadata_view metadata() const noexcept;

    u32_t addr_info_size() const noexcept;

    [[nodiscard]] strings::const_string_view label() const noexcept;
  };

  struct addr_info_iterator final {
    const addr_info * it;

    J_INLINE_GETTER const addr_info * operator->() const noexcept {
      J_ASSUME_NOT_NULL(it);
      return it;
    }

    J_INLINE_GETTER const addr_info & operator*() const noexcept {
      J_ASSUME_NOT_NULL(it);
      return *it;
    }

    addr_info_iterator & operator++() noexcept {
      it = add_bytes(it, it->addr_info_size());
      return *this;
    }

    addr_info_iterator operator++(int) noexcept;

    J_INLINE_GETTER bool operator==(const addr_info_iterator &) const noexcept = default;
  };

  struct addr_info_view final {
    const addr_info * const m_begin;
    const addr_info * const m_end;

    J_INLINE_GETTER addr_info_iterator begin() const noexcept {
      return { m_begin };
    }

    J_INLINE_GETTER addr_info_iterator end() const noexcept {
      return { m_end };
    }
  };
}
