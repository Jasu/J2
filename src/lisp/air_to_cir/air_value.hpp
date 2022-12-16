#pragma once

#include "lisp/cir/ops/input.hpp"
#include "lisp/air/values/rep.hpp"
#include "lisp/values/lisp_imms.hpp"

namespace j::lisp::air_to_cir {
  enum range_type : u8_t {
    plain_range = imm_range,
    vec_range   = imm_vec_ref,
  };

  /// Representation of values handled when converting to CIR.
  struct J_TYPE_HIDDEN air_value final {
    J_BOILERPLATE(air_value, CTOR_CE)

    template<typename Value>
    constexpr air_value(Value && value, imm_type_mask types, air::rep rep) noexcept
      : types(types),
        rep(rep),
        value(static_cast<Value &&>(value))
    {
    }

    template<typename Value, typename Sz>
    constexpr air_value(Value && value, imm_type_mask types, air::rep rep, Sz && size) noexcept
    : types(types),
      rep(rep),
      value(static_cast<Value &&>(value)),
      size(static_cast<Sz&&>(size))
    {
    }

    void constrain_to_types(imm_type_mask constraint) noexcept {
      types &= constraint;
    }

    J_A(AI,ND,NODISC) bool is_const() const noexcept {
      return value.is_constant();
    }

    [[nodiscard]] inline u64_t const_val() const noexcept {
      J_ASSERT(value.is_constant());
      return value.const_data;
    }

    template<typename Ptr, typename Sz>
    air_value(range_type type, Ptr && ptr, Sz && sz) noexcept
      : types(type),
        rep(air::rep_range),
        value(static_cast<Ptr &&>(ptr)),
        size(static_cast<Sz &&>(sz))
    {
    }

    J_INLINE_GETTER imm_type only_type() const {
      return types.only_value();
    }

    J_INLINE_GETTER bool is_tagged() const noexcept {
      return rep == air::rep_tagged;
    }

    J_INLINE_GETTER bool is_untagged() const noexcept {
      return rep == air::rep_untagged;
    }

    J_INLINE_GETTER bool is_nil() const noexcept {
      return types.only_value() == imm_nil;
    }

    J_INLINE_GETTER bool is_range() const noexcept {
      return rep == air::rep_range;
    }

    imm_type_mask types = {};
    air::rep rep = air::rep_none;

    cir::input value;
    cir::input size;

    J_A(AI,ND,NODISC) air_value range_length() const noexcept {
      return air_value(size, imm_i64, air::rep_untagged);
    }

    J_A(AI,ND,NODISC) explicit operator bool() const noexcept {
      return rep != air::rep_none;
    }

    J_A(AI,ND,NODISC) bool operator!() const noexcept {
      return rep == air::rep_none;
    }

    J_A(AI,ND,NODISC) bool empty() const noexcept {
      return rep == air::rep_none;
    }

    J_A(AI,ND,NODISC) bool has_size() const noexcept {
      return (bool)size;
    }
  };
}
