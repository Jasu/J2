#pragma once

#include "lisp/cir/ssa/bb_builder.hpp"
#include "lisp/common/truthiness.hpp"
#include "lisp/air_to_cir/result_use.hpp"
#include "lisp/air_to_cir/air_value.hpp"
#include "lisp/air/exprs/expr.hpp"
#include "lisp/cir/ops/ops.hpp"
#include "lisp/air/air_package.hpp"

namespace j::lisp::air_to_cir {
  namespace c = cir;

  inline const air_value empty_vec_untagged{
    c::var_value_untagged_reloc(air::static_ids::id_empty_vec),
    imm_vec_ref,
    air::rep_untagged,
    c::untagged_const_int(0)
  };

  inline const air_value empty_vec_range{
    c::var_value_untagged_reloc(air::static_ids::id_empty_vec, 8U),
    imm_vec_ref,
    air::rep_range,
    c::untagged_const_int(0)
  };

  inline const air_value empty_vec_tagged{
    c::var_value_tagged_reloc(air::static_ids::id_empty_vec),
    imm_vec_ref,
    air::rep_tagged,
    c::untagged_const_int(0)
  };

  inline constexpr auto air_value_none = air_value{};
  inline const auto air_value_nil = air_value(c::const_nil, imm_nil, air::rep_tagged);
  inline const auto air_value_true = air_value(c::const_true, imm_bool, air::rep_tagged);
  inline const auto air_value_false = air_value(c::const_false, imm_bool, air::rep_tagged);

  J_INLINE_GETTER constexpr const air_value & air_bool(bool val) noexcept {
    return val ? air_value_true : air_value_false;
  }

  template<typename Val, typename... Sz>
  J_INLINE_GETTER air_value tagged_vec(Val && val, Sz && ... sz) noexcept {
    return air_value(static_cast<Val &&>(val), imm_vec_ref, air::rep_tagged, static_cast<Sz &&>(sz)...);
  }

  template<typename Val, typename... Sz>
  [[nodiscard]] inline air_value untagged_vec(Val && val, Sz && ... sz) noexcept {
    return air_value(static_cast<Val &&>(val), imm_vec_ref, air::rep_untagged, static_cast<Sz &&>(sz)...);
  }

  template<typename Val>
  [[nodiscard]] inline air_value untagged_i64(Val && val) noexcept {
    return air_value(static_cast<Val &&>(val), imm_i64, air::rep_untagged);
  }

  template<Integral Val>
  J_A(AI,NODISC,ND,HIDDEN) air_value untagged_i64(Val val) noexcept {
    return air_value(cir::untagged_const_int(val), imm_i64, air::rep_untagged);
  }

  template<typename... Sz>
  [[nodiscard]] inline air_value wrap_expr_op(const air::exprs::expr * J_NOT_NULL e,
                                         cir::op * J_NOT_NULL op,
                                         Sz && ... sz) noexcept {
    return air_value(
      op,
      e->result.types,
      e->rep(),
      static_cast<Sz &&>(sz)...);
  }

  [[nodiscard]] inline air_value maybe_tag(cir::bb_builder & bb, const air::exprs::expr * J_NOT_NULL e, air_value value, result_use use, air::rep target_rep = air::rep_tagged) {
    if (use == result_unused || use == size_used || use == bool_used || value.is_tagged() || target_rep != air::rep_tagged ) {
      return value;
    }
    J_ASSERT(value.is_untagged());
    const auto type = value.only_type();
    if (value.value.is_reloc()) {
      switch (value.value.reloc_data.reloc_type) {
      case c::reloc_type::none:
        J_FAIL("Empty reloc");
      case c::reloc_type::var_addr:
        switch (type) {
        case imm_vec_ref:
          return {
            value.value.reloc_data.with_addend((i32_t)tag_vec_ref),
            imm_vec_ref,
            air::rep_tagged
          };
          break;
        case imm_fn_ref:
          return {
            value.value.reloc_data.with_addend((i32_t)tag_fn_ref),
            imm_fn_ref,
            air::rep_tagged
          };
          break;
        case imm_closure_ref:
          return {
            value.value.reloc_data.with_addend((i32_t)tag_closure_ref),
            imm_closure_ref,
            air::rep_tagged
          };
          break;
        case imm_str_ref:
          return {
            value.value.reloc_data.with_addend((i32_t)tag_str_ref),
            imm_str_ref,
            air::rep_tagged
          };
          break;
        default:
          J_FAIL("Unsupported reloc value type");
        }
      case c::reloc_type::var_value_tagged:
        J_FAIL("Reloc was tagged");
      case c::reloc_type::var_value_untagged:
        return {
          value.value.reloc_data.with_type(c::reloc_type::var_value_tagged),
          imm_fn_ref,
          air::rep_tagged,
          value.size,
        };
      case c::reloc_type::constant_addr:
        J_TODO();
      case c::reloc_type::fn_addr:
        return {
          value.value.reloc_data.with_addend((i32_t)tag_fn_ref),
          imm_fn_ref,
          air::rep_tagged
        };
      }
    }

    if (value.is_const()) {
      i64_t const_val = (i64_t)value.const_val();
      u32_t size = 0U;
      switch (type) {
      case imm_undefined: J_FAIL("Undef lisp_imm in tag.");
      case imm_sym_id:
      case imm_bool:
      case imm_nil:
      case imm_range:
      case imm_act_rec:
        J_FAIL("Untaggable type.");
      case imm_f32:
      case imm_rec_ref:
        J_TODO();
      case imm_fn_ref:
        const_val += (i64_t)tag_fn_ref;
        break;
      case imm_closure_ref:
        const_val += (i64_t)tag_closure_ref;
        break;
      case imm_vec_ref:
        size = reinterpret_cast<lisp_vec*>((uptr_t)const_val)->size();
        const_val += (i64_t)tag_vec_ref;
        break;
      case imm_str_ref:
        const_val += (i64_t)tag_str_ref;
        break;
      case imm_i64:
        const_val <<= 1U;
        break;
      }
      return air_value(cir::tagged_raw_const(const_val, type), type, air::rep_tagged, cir::untagged_const_int(size));
    }

    return air_value(
      bb.emplace_back(cir::ops::defs::tag,
                      copy_metadata(e->metadata()),
                      type, value.value),
      value.types,
      air::rep_tagged,
      value.size
    );
  }

  [[nodiscard]] inline air_value maybe_untag(cir::bb_builder & bb, const air::exprs::expr * J_NOT_NULL e, air_value value, result_use use = result_used, air::rep target_rep = air::rep_untagged) {
    if (use == result_unused || use == size_used || use == bool_used || value.is_untagged() || target_rep != air::rep_untagged) {
      return value;
    }
    J_ASSERT(value.is_tagged());
    const auto type = value.only_type();
    if (value.is_const()) {
      i64_t const_val = (i64_t)value.const_val();
      u32_t size = 0U;
      switch (type) {
      case imm_undefined: J_FAIL("Undef lisp_imm in untag.");
      case imm_sym_id:
      case imm_bool:
      case imm_nil:
      case imm_act_rec:
      case imm_range:
        J_FAIL("Un-untaggable type.");
      case imm_f32:
      case imm_rec_ref:
        J_TODO();
      case imm_fn_ref:
        const_val -= (i64_t)tag_fn_ref;
        break;
      case imm_closure_ref:
        const_val -= (i64_t)tag_closure_ref;
        break;
      case imm_vec_ref:
        const_val -= (i64_t)tag_vec_ref;
        size = reinterpret_cast<lisp_vec*>((uptr_t)const_val)->size();
        break;
      case imm_str_ref:
        const_val -= (i64_t)tag_str_ref;
        break;
      case imm_i64:
        const_val >>= 1U;
        break;
      }
      return air_value(cir::untagged_raw_const(const_val, type), value.types, air::rep_untagged, cir::untagged_const_int(size));
    }

    return air_value(
      bb.emplace_back(cir::ops::defs::untag, copy_metadata(e->metadata()), type, value.value),
      value.types,
      air::rep_untagged,
      value.size
    );
  }

  [[nodiscard]] inline air_value air_constant(cir::bb_builder & bb, const air::exprs::expr * J_NOT_NULL e, lisp_imm imm, air::rep rep, result_use use) {
    if (use == result_unused) {
      return air_value_none;
    }
    namespace o = j::lisp::cir::ops::defs;
    u64_t raw = imm.raw;
    if (use == bool_used) {
      return air_bool(raw != false_v && raw != nil_v);
    }
    auto type = imm.type();
    auto lmd = copy_metadata(e->metadata());
    switch (rep) {
    case air::rep_none:
      J_UNREACHABLE();
    case air::rep_tagged:
      if (type == imm_vec_ref) {
        u32_t sz = imm.as_vec_ref()->size();
        if (!sz) {
          return empty_vec_tagged;
        }
        if (use == size_used) {
          return air_value(cir::const_nil, type, rep, cir::untagged_const_int(sz));
        }
        return tagged_vec(bb.emplace_back(o::iconst, lmd, raw), cir::untagged_const_int(sz));
      }
      if (raw < (1U << 31)) {
        return air_value(cir::tagged_raw_const(raw, type), type, rep);
      }
      return air_value(bb.emplace_back(o::iconst, lmd, raw), type, rep);
    case air::rep_untagged:
      switch (type) {
      case imm_undefined: J_FAIL("Undef lisp_imm in constant.");
      case imm_i64: {
        i64_t val = imm.as_i64().value();
        return air_value(cir::untagged_const_int(val), type, rep);
      }
      case imm_vec_ref: {
        u32_t sz = imm.as_vec_ref()->size();
        if (!sz) {
          return empty_vec_untagged;
        }
        if (use == size_used) {
          return air_value(cir::const_nil, type, rep, cir::untagged_const_int(sz));
        }
        return untagged_vec(bb.emplace_back(o::iconst, lmd, imm.raw - (u64_t)tag_vec_ref), cir::untagged_const_int(sz));
      }
      case imm_closure_ref:
        return air_value(bb.emplace_back(o::iconst, lmd, imm.raw - (u64_t)tag_closure_ref), type, rep);
      case imm_fn_ref:
        return air_value(bb.emplace_back(o::iconst, lmd, imm.raw - (u64_t)tag_fn_ref), type, rep);
      case imm_str_ref:
        return air_value(bb.emplace_back(o::iconst, lmd, imm.raw - (u64_t)tag_str_ref), type, rep);
      case imm_rec_ref:
        return air_value(bb.emplace_back(o::iconst, lmd, imm.raw - (u64_t)tag_rec_ref), type, rep);
      case imm_f32:
        J_TODO();
      case imm_range:
      case imm_sym_id:
      case imm_bool:
      case imm_nil:
      case imm_act_rec:
        J_FAIL("Unsupported for untagged");
      }
    case air::rep_range: {
      auto vec = imm.as_vec_ref()->value();
      const u32_t sz = vec.size();
      return air_value(
        plain_range,
        bb.emplace_back(o::mconst, lmd, j::mem::const_memory_region(vec.begin(), sz * sizeof(lisp_imm))),
        cir::untagged_const_int(sz));
    }
    }
  }

  [[nodiscard]] inline truthiness get_truthiness(const air_value & val) noexcept {
    if (val.is_const()) {
      const auto c = val.const_val();
      return (c == false_v || c == nil_v) ? truthiness::always_falsy : truthiness::always_truthy;
    }
    return type_truthiness(val.types);
  }
}
