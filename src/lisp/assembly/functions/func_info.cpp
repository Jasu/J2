#include "lisp/assembly/functions/func_info.hpp"

#include <libunwind.h>

namespace j::lisp::assembly::inline functions {
  namespace {
    // A dummy dynamic unwind table. libunwind documentation recommends using
    // this format, and it is the only documented unwind table format. Too bad
    // it is only supported on actual Itanium, which the docs fail to mmention.
    const unw_dyn_region_info_t dummy_region{
      .next = nullptr,
      .insn_count = 1,
      .op_count = 1U,
      .op = {{
        .tag = UNW_DYN_STOP,
        .qp = _U_QP_TRUE,
        .reg = -1,
        .when = 0,
        .val = 0U,
        }
      },
    };
  }

  void func_info::register_unwind(void * J_NOT_NULL fn_start) {
    J_ASSUME(!unwind_data);
    unwind_data = ::j::allocate(sizeof(unw_dyn_info_t));
    *(unw_dyn_info_t*)unwind_data = {
      .next = nullptr,
      .prev = nullptr,
      .start_ip = (unw_word_t)fn_start,
      .end_ip = (unw_word_t)add_bytes(fn_start, func_size),
      .gp = 0U,
      .format = UNW_INFO_FORMAT_DYNAMIC,
      .u = {
        .pi = {
          .name_ptr = (unw_word_t)func_name().begin(),
          .handler = 0U,
          .flags = 0U,
          .regions = const_cast<unw_dyn_region_info_t*>(&dummy_region),
        },
      },
    };
    _U_dyn_register((unw_dyn_info_t*)unwind_data);
  }

  void func_info::unregister_unwind() noexcept {
    J_ASSUME_NOT_NULL(unwind_data);
    _U_dyn_cancel((unw_dyn_info_t*)unwind_data);
    ::j::free(unwind_data);
    unwind_data = nullptr;
  }

  span<const trap_info> func_info::traps() const noexcept {
    return { reinterpret_cast<const trap_info*>(this + 1), num_traps };
  }

  strings::const_string_view func_info::func_name() const noexcept {
    return { add_bytes<const char *>(this + 1, num_traps * sizeof(trap_info)), func_name_size };
  }

  [[nodiscard]] addr_info_view func_info::addr_info() const noexcept {
    const auto beg = add_bytes<const struct addr_info*>(
      this + 1,
      align_up(num_traps * sizeof(trap_info) + func_name_size + 1U, 8U)
      );
    return {
      beg,
      add_bytes(beg, addr_info_size),
    };
  }
}
