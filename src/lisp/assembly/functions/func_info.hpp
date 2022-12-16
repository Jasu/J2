#pragma once

#include "lisp/assembly/functions/trap_info.hpp"
#include "lisp/assembly/functions/addr_info.hpp"
#include "containers/span.hpp"
#include "strings/string_view.hpp"

namespace j::lisp::assembly::inline functions {
  struct alignas(u64_t) func_info final {
    u16_t num_traps = 0U;
    u16_t addr_info_size = 0U;
    u16_t func_name_size = 0U;
    /// Size of function code, in bytes.
    u32_t func_size = 0U;
    /// Platform-specific unwind registration, e.g. a libunwind's `unw_dyn_info_t`.
    void * unwind_data = nullptr;

    void register_unwind(void * J_NOT_NULL fn_start);
    void unregister_unwind() noexcept;

    [[nodiscard]] span<const trap_info> traps() const noexcept;

    [[nodiscard]] strings::const_string_view func_name() const noexcept;

    [[nodiscard]] addr_info_view addr_info() const noexcept;
  };
}
