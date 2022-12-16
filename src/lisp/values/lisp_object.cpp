#include "lisp/values/lisp_object.hpp"
#include "lisp/mem/heap_cell.hpp"
#include "lisp/values/lisp_act_record.hpp"

namespace j::lisp::inline values {
  J_A(HIDDEN) lisp_imm::lisp_imm(null_t&) noexcept { }


  [[nodiscard]] sources::source_location lisp_object::get_source_location() {
    if (!header.object_hightag().has_debug_info()) {
      return sources::source_location{};
    }
    lisp::mem::heap_cell * prev_footer = reinterpret_cast<lisp::mem::heap_cell *>(this - 1);
    if (!prev_footer->is_footer()) {
      J_TODO();
    }
    lisp::mem::heap_cell * dbg_ = prev_footer->as_footer().start();
    J_ASSERT(dbg_ && dbg_->is_debug_info());
    auto & dbg = dbg_->as_debug_info();
    if (dbg.debug_info_type() == lisp::mem::debug_info_type::source_location) {
      return dbg.source_location();
    }
    return sources::source_location{};
  }

  lisp_vec::lisp_vec(const const_imms_t & value, lisp::mem::object_hightag_flag flags,
                     u8_t ext_refs) noexcept
    : lisp_object(lisp::mem::vec_tag, value.size(), flags, ext_refs)
  {
    if (const u32_t sz = value.size()) {
      ::j::memcpy(lisp_object::data<void>(), value.begin(), sz * 8U);
    }
  }

  lisp_str::lisp_str(strings::const_string_view value, lisp::mem::object_hightag_flag flags, u8_t ext_refs) noexcept
      : lisp_str(value.size(), flags, ext_refs)
    {
      if (value) {
        ::j::memcpy(data(), value.begin(), value.size());
        if (value.size() & 0x7U) {
          ::j::memzero(add_bytes(data(), value.size()), 0x8U - 0x7U & value.size());
        }
      }
    }
}
