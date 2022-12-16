#include "lisp/assembly/buffers/buffer.hpp"

namespace j::lisp::assembly::inline buffers {
  bool buffer::operator==(const buffer & rhs) const noexcept {
    if (relocs.size() != rhs.relocs.size() || data != rhs.data) {
      return false;
    }
    for (u32_t i = 0; i < relocs.size(); ++i) {
      if (relocs[i] != rhs.relocs[i]) {
        return false;
      }
    }
    return true;
  }

  void buffer::set_base(u32_t base) noexcept {
    for (auto & r : relocs) {
      r.set_base(base);
    }
  }
}
