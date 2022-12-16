#include "lisp/mem/heap_ctrl_value.hpp"
#include "lisp/mem/heap_cell.hpp"

namespace j::lisp::mem {

  [[nodiscard]] u32_t heap_ctrl_value::size_taken() const noexcept {
    const u32_t d = header.lowtag.data();
    switch(header.lowtag.type()) {
    case lowtag_type::sentinel:
    case lowtag_type::footer:
      return 8U;
    case lowtag_type::free_chunk:
      return 8U * d;
    case lowtag_type::debug_info:
      switch ((debug_info_type)d) {
      case debug_info_type::source_location:
        return 16U + align_up(sizeof(sources::source_location), 8U);
      }
    case lowtag_type::str:
    case lowtag_type::vec:
    case lowtag_type::act_record:
      J_FAIL("Lisp object as heap_ctrl_value");
    }
  }
  heap_debug_info::heap_debug_info(enum debug_info_type type) noexcept
    : heap_ctrl_value(debug_info_tag, type)
  { }

  heap_debug_info::heap_debug_info(detail::source_location_tag_t, const sources::source_location & loc) noexcept
    : heap_ctrl_value(debug_info_tag, debug_info_type::source_location)
  {
    ::new (this + 1) sources::source_location(loc);
  }

  [[nodiscard]] sources::source_location & heap_debug_info::source_location() noexcept {
    J_ASSERT(debug_info_type() == debug_info_type::source_location);
    return *data<sources::source_location>();
  }

  [[nodiscard]] heap_cell * heap_cell::next() noexcept {
    if (is_sentinel() && header.lowtag_data() == 1U) {
      return nullptr;
    }
    return add_bytes(this, size_taken());
  }
}
