#pragma once

#include "containers/vector.hpp"
#include "strings/string_map_fwd.hpp"
#include "meta/attr_value.hpp"

namespace j::meta {
  struct expr_scope;

  enum arg_mode_flags : u8_t {
    arg_required         = 0b0000U,
    arg_optional         = 0b0001U,
    arg_accept_array     = 0b0010U,
    arg_only_array       = 0b0100U,
    arg_array            = 0b0110U,
    arg_rest             = 0b1000U
  };

  enum arg_init_flags : u8_t {
    required           = arg_required,
    rest               = arg_rest | arg_optional,
    lax_rest           = rest | arg_accept_array,
  };

  struct arg_init final {
    attr_type_mask types = attr_mask_none;
    arg_mode_flags flags = arg_required;

    J_A(AI,ND,HIDDEN) constexpr inline arg_init(attr_type_mask types, bool is_array, arg_init_flags flags) noexcept
      : types(types),
        flags((arg_mode_flags)(
                (flags & 0b1111U)
                | ((is_array && !(flags & (u8_t)arg_rest))
                   ? arg_only_array : 0U)))
    {
    }

    J_A(AI,ND) constexpr inline bool is_required() const noexcept {
      return !(flags & (u8_t)arg_optional);
    }
    J_A(AI,ND) constexpr inline bool is_rest() const noexcept {
      return flags & (u8_t)arg_rest;
    }
  };

  struct arg_settings final {
    attr_type_mask types = attr_mask_none;
    arg_mode_flags flags = arg_required;

    J_A(ND,NODISC,AI,HIDDEN) inline bool accepts_array() const noexcept {
      return flags & arg_accept_array;
    }

    J_A(ND,NODISC,AI,HIDDEN) inline bool accepts_single() const noexcept {
      return !(flags & arg_only_array);
    }

    J_A(ND,NODISC,AI,HIDDEN) inline bool is_optional() const noexcept {
      return flags & arg_optional;
    }

    J_A(ND,NODISC,AI,HIDDEN) inline bool is_rest() const noexcept {
      return flags & arg_rest;
    }
  };

  struct expr_fn;

  using expr_fn_ptr_t = attr_value (*)(const expr_fn &, expr_scope &, attr_value *, u32_t);

  struct expr_fn final {
    attr_type_mask arg_types[4U]{attr_mask_none};
    arg_mode_flags arg_flags[4U]{arg_required};
    u8_t num_args = 0U;
    u8_t num_required_args = 0U;

    J_A(NODISC,ND,AI) inline arg_settings arg_settings_at(u8_t i) const noexcept {
      return {arg_types[i], arg_flags[i]};
    }

    expr_fn_ptr_t invoke = nullptr;
    void * userdata = nullptr;

    [[nodiscard]] bool can_invoke(const attr_value * args, u32_t nargs) const;

    J_A(AI,ND,HIDDEN) inline attr_value operator()(expr_scope & scope, attr_value * args, u32_t nargs) const {
      return invoke(*this, scope, args, nargs);
    }
  };
}

J_DECLARE_EXTERN_STRING_MAP(j::noncopyable_vector<j::meta::expr_fn>);

namespace j::meta {
  struct fn_collection final {
    strings::string_map<noncopyable_vector<expr_fn>> functions;

    [[nodiscard]] const expr_fn * maybe_get_fn(strings::const_string_view name, const attr_value * args, u32_t nargs);
    fn_collection & add_fn(strings::const_string_view name, expr_fn value) noexcept;
  };

  [[nodiscard]] fn_collection make_root_fn_collection() noexcept;
}
