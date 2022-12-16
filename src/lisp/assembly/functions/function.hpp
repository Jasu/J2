#pragma once

#include "lisp/assembly/functions/func_info.hpp"
#include "lisp/assembly/buffers/buffer.hpp"

namespace j::lisp::assembly::inline functions {
  struct function final {
    buffer code;
    buffer constants;
    func_info * info = nullptr;

    span<const trap_info> traps() const noexcept {
      J_ASSUME_NOT_NULL(info);
      return info->traps();
    }

    strings::const_string_view func_name() const noexcept {
      J_ASSUME_NOT_NULL(info);
      return info->func_name();
    }

    addr_info_view addr_info() const noexcept {
      J_ASSUME_NOT_NULL(info);
      return info->addr_info();
    }
  };
}
