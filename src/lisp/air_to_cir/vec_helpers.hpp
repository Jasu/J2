#pragma once

#include "lisp/air_to_cir/value_helpers.hpp"

namespace j::lisp::air_to_cir {
  namespace {
    namespace c = cir;
    namespace o = cir::ops::defs;

    [[maybe_unused]] inline constexpr const air_value & get_empty_vec(air::rep rep) noexcept {
      switch (rep) {
      case air::rep_tagged:
        return empty_vec_tagged;
      case air::rep_untagged:
        return empty_vec_untagged;
      case air::rep_range:
        return empty_vec_range;
      case air::rep_none:
        J_UNREACHABLE();
      }
    }

    [[maybe_unused]] [[nodiscard]] inline c::input range_ptr(air_value rng, air_value index) {
      J_ASSERT((rng.types == imm_vec_ref || rng.types == imm_range) && !rng.is_const() && index.types == imm_i64,
               "range_ptr failure");
      return c::scaled_qword_ptr(
        rng.value,
        index.value,
        // Instead of untagging the value, use a smaller scale
        index.is_tagged() ? mem_scale::scale_4 : mem_scale::scale_8,
        rng.is_range() ? 0U : (rng.is_tagged() ? 7U : 8U));
    }

    [[maybe_unused]] inline air_value get_range_begin(cir::bb_builder & bb, const air::exprs::expr * e, air_value rng) {
      if (rng.is_range()) {
        return untagged_i64(rng.value);
      }
      J_ASSERT(rng.types == imm_vec_ref);
      return untagged_i64(bb.emplace_back(o::iadd, copy_metadata(e->metadata()), rng.value, c::untagged_const_int(rng.is_tagged() ? 7U : 8U)));
    }

    [[maybe_unused]] J_A(AI,ND,NODISC) inline c::input vec_size_ptr(air_value vec) {
      J_ASSERT(vec.types == imm_vec_ref);
      return c::dword_ptr(vec.value, vec.is_tagged() ? -1 : 0);
    }

    [[maybe_unused]] inline c::input range_length_input(air_value input) {
      if (input.has_size()) {
        return input.size;
      }
      return vec_size_ptr(input);
    }

    [[maybe_unused]] inline air_value get_range_length(cir::bb_builder & bb, const air::exprs::expr * e, air_value input, bool bytes_with_offset = false) {
      auto md = copy_metadata(e->metadata());
      air_value size = input.has_size() ? input.range_length() : untagged_i64(bb.emplace_back(cir::ops::defs::mem_ird, md, vec_size_ptr(input)));
      if (bytes_with_offset) {
        if (size.is_const()) {
          return untagged_i64(size.const_val() * 8U + 8U);
        }
        size = untagged_i64(bb.emplace_back(cir::ops::defs::shl, md, size.value, c::untagged_const_int(3U)));
        size = untagged_i64(bb.emplace_back(cir::ops::defs::iadd, md, size.value, c::untagged_const_int(8U)));
      }
      return size;
    }
  }
}
