#pragma once

#include "lisp/cir/locs/loc_specifier.hpp"
#include "lisp/cir/ops/op_flags.hpp"
#include "lisp/cir/ops/op_type.hpp"

namespace j::lisp::cir::inline ops {
  struct op;

  /// Number of param info slots.
  ///
  /// Seven are required, since AMD64 ABI uses six GPR registers
  /// for parameter passing, and one slot is required for the function
  /// address for the `call` op.
  constexpr inline i32_t param_info_count_v J_A(ND) = 7;
  constexpr inline i32_t static_info_count_v J_A(ND) = 3;
  constexpr inline i32_t aux_reg_info_count_v J_A(ND) = 1;

  struct result_data final {
    loc_specifier (*spec_getter)(const op * J_NOT_NULL) noexcept = nullptr;
    loc_specifier spec = loc_specifier::none;
    alias_flags alias = alias_any;
    type_mode type = type_mode::any;

    consteval result_data() noexcept = default;

    bool is_fp() const noexcept {
      return type == type_mode::fp;
    }

    explicit consteval result_data(const loc_specifier & spec, type_mode type, alias_flags alias) noexcept
      : spec(spec),
        alias(alias),
        type(type)
    { }

    explicit consteval result_data(loc_specifier (*getter)(const op * J_NOT_NULL) noexcept, type_mode type, alias_flags alias) noexcept
      : spec_getter(getter),
        alias(alias),
        type(type)
    { }

    J_A(AI,NODISC) inline loc_specifier get_loc_specifier(const op * J_NOT_NULL op) const noexcept {
      return spec_getter ? spec_getter(op) : spec;
    }
  };

  struct param_data final {
    const char * name = nullptr;
    loc_specifier spec = loc_specifier::none;
    bool allow_const = false;
    type_mode type = type_mode::any;
    alias_flags alias_flags = alias_any;
  };

  struct aux_reg_data final {
    const char * name = nullptr;
    loc_specifier spec = loc_specifier::none;
    type_mode type = type_mode::any;
    alias_flags alias_flags = alias_any;
  };

  struct op_data final {
    result_data result{};
    param_data params[param_info_count_v]{};
    const char * statics[static_info_count_v]{nullptr};
    aux_reg_data aux_regs[aux_reg_info_count_v]{};
    loc_mask clobbered_regs = loc_mask::none;

    J_A(AI,NODISC) inline loc_specifier result_specifier(const op * J_NOT_NULL o) const noexcept {
      return result.get_loc_specifier(o);
    }
  };

  struct op_data_table final {
    op_data data[max_op_type_v + 1];

    J_A(AI,NODISC) inline op_data & operator[](op_type t) noexcept {
      return data[(u8_t)t];
    }

    J_A(AI,NODISC) inline const op_data & operator[](op_type t) const noexcept {
      return data[(u8_t)t];
    }

    J_A(AI,NODISC) inline loc_specifier result_specifier(op_type t, const op * J_NOT_NULL o) const noexcept {
      return data[(u8_t)t].result.get_loc_specifier(o);
    }

    J_A(AI,NODISC) inline loc_specifier arg_specifier(op_type t, i32_t i) const noexcept {
      return data[(u8_t)t].params[J_MIN(i, param_info_count_v - 1)].spec;
    }

    J_A(AI,NODISC) inline const char * arg_name(op_type t, i32_t i) const noexcept {
      return data[(u8_t)t].params[J_MIN(i, param_info_count_v - 1)].name;
    }

    J_A(AI,NODISC) inline loc_specifier aux_specifier(op_type t, i32_t i) const noexcept {
      return data[(u8_t)t].aux_regs[J_MIN(i, aux_reg_info_count_v - 1)].spec;
    }

    J_A(AI,NODISC) inline bool is_aux_fp(op_type t, u32_t i) const noexcept {
      return data[(u8_t)t].aux_regs[J_MIN(i, aux_reg_info_count_v - 1)].type == type_mode::fp;
    }
  };

  extern constinit const op_data_table default_op_data;
}
