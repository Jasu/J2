#include "lisp/values/imm_type.hpp"

J_DEFINE_EXTERN_FLAGS(j::lisp::values::imm_type, u16_t);

namespace j::lisp::inline values {
  namespace {
    constexpr inline imm_tag imm_tags[] = {
      tag_i64,
      tag_f32,
      tag_bool,
      tag_nil,
      tag_sym_id,

      tag_fn_ref,
      tag_closure_ref,
      tag_str_ref,
      tag_vec_ref,
      tag_rec_ref,

      tag_undefined,
    };
  }

  J_A(CONST,NODISC) imm_tag imm_tag_by_type(imm_type idx) noexcept {
    J_ASSERT(idx <= imm_max_tagged);
    return imm_tags[(u8_t)idx];
  }

  J_A(NODISC,CONST) imm_type imm_type_by_tag(imm_tag t) noexcept {
    for (u32_t i = 0; i < J_ARRAY_SIZE(imm_tags); ++i) {
      if (imm_tags[i] == t) {
        return (imm_type)i;
      }
    }
    J_FAIL("Immediate type not found.");
  }

  J_A(NODISC,CONST) imm_tag tag_of(u8_t tagged) noexcept {
    if (!(tagged & 1U)) {
      return tag_i64;
    }
    if ((tagged & 0b111U) != 0b111U) {
      return (imm_tag)(tagged & 0b111U);
    }
    return tagged == tag_undefined ? tag_undefined : (imm_tag)(tagged & 0b111111U);
  }

  J_A(NODISC,CONST) truthiness type_truthiness(const imm_type_mask & m) noexcept {
    if (m.value & ((1U << imm_nil) | (1U << imm_bool))) {
      return m.value == (1U << imm_nil) ? truthiness::always_falsy : truthiness::unknown;
    }
    return truthiness::always_truthy;
  }
}
