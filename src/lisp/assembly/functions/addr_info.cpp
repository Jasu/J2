#include "lisp/assembly/functions/addr_info.hpp"
#include "logging/global.hpp"

namespace j::lisp::assembly::inline functions {
  metadata_view addr_info::metadata() const noexcept {
    return {
      has_metadata ?
      align_up(add_bytes<const struct metadata*>(this + 1, label_size), 8)
      : nullptr
    };
  }

  [[nodiscard]] strings::const_string_view addr_info::label() const noexcept {
    return { (const char *)(this + 1), label_size };
  }

  u32_t addr_info::addr_info_size() const noexcept {
    const struct metadata * md = align_up(add_bytes<const struct metadata*>(this + 1, label_size), 8U);
    if (has_metadata) {
      while (!md->is_last) {
        md = md->next();
      }
    }
    return align_up(byte_offset(this, md) + (has_metadata ? md->size() : 0), 8);
  }

  addr_info_iterator addr_info_iterator::operator++(int) noexcept {
    auto result = *this;
    ++result;
    return result;
  }
}
