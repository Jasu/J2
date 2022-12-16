#pragma once

#include "lisp/cir/ops/op_type.hpp"

namespace j::lisp::cir::inline debug {
  /// Barrier types used for sequencing the CIR graph.
  enum class barrier_type : u8_t {
    none,
    wr,
    rd,
    call,
    error,
    max_barrier = error,
  };

  constexpr u8_t num_barrier_types = (u8_t)barrier_type::max_barrier + 1U;

  /// Backgraound colors for each barrier type.
  inline constexpr const char * barrier_type_colors[num_barrier_types] = {
    [(u8_t)barrier_type::none] = "#D8D8E8",
    [(u8_t)barrier_type::wr] = "#FFE880",
    [(u8_t)barrier_type::rd] = "#9AFE9A",
    [(u8_t)barrier_type::call] = "#F060FF",
    [(u8_t)barrier_type::error] = "#FFAA44",
  };

  /// Information for presenting an instruction.
  struct J_TYPE_HIDDEN op_debug_info final {
    barrier_type barrier_type = barrier_type::none;

    J_INLINE_GETTER_NONNULL const char * barrier_color() const noexcept {
      return barrier_type_colors[(u8_t)barrier_type];
    }
  };

#define OP_DBG_WR .barrier_type = barrier_type::wr
#define OP_DBG_RD .barrier_type = barrier_type::rd
#define OP_DBG_CALL .barrier_type = barrier_type::call
#define OP_DBG_ERR .barrier_type = barrier_type::error

#define OP_DBG_APPLY(...) OP_DBG_##__VA_ARGS__
#define OP_DBG_INFO(T, ...)                               \
  [(u8_t)op_type::T] = {J_FEC(OP_DBG_APPLY, __VA_ARGS__)}

  inline constexpr op_debug_info op_debug_data[num_op_types_v] = {
    OP_DBG_INFO(mem_iwr, WR),
    OP_DBG_INFO(mem_fwr, WR),
    OP_DBG_INFO(mem_copy64, WR),

    OP_DBG_INFO(mem_ird, RD),
    OP_DBG_INFO(mem_frd, RD),

    OP_DBG_INFO(full_call, CALL),
    OP_DBG_INFO(abi_call, CALL),
    OP_DBG_INFO(call, CALL),

    OP_DBG_INFO(debug_trap, ERR),
    OP_DBG_INFO(type_error, ERR),
  };
}
