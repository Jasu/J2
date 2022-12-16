#pragma once

#include "lisp/air/values/rep.hpp"
#include "lisp/values/imm_type.hpp"

namespace j::lisp::air::inline values {
  struct J_PACKED val_spec final {
    imm_type_mask types = {};
    rep_spec reps = {};

    [[nodiscard]] inline enum rep rep() const noexcept {
      J_ASSERT(reps.rep() != rep_none);
      return reps.rep();
    }

    J_INLINE_GETTER imm_type type() const noexcept {
      return types.only_value();
    }

    J_INLINE_GETTER bool is_tagged() const noexcept {
      return rep() == rep_tagged;
    }

    J_INLINE_GETTER bool is_untagged() const noexcept {
      return rep() == rep_untagged;
    }

    J_A(AI,NODISC) inline bool operator==(const val_spec &) const noexcept = default;
  };

  inline constexpr const val_spec val_none J_A(HIDDEN) {};
  inline constexpr const val_spec val_any J_A(HIDDEN) {any_imm_type,               rep_spec::any};

  inline constexpr const val_spec val_range J_A(HIDDEN) {range_imm_type,           rep_spec::range};
  inline constexpr const val_spec val_vec_or_range J_A(HIDDEN) {range_imm_type,    rep_spec::any};

  inline constexpr const val_spec val_vec J_A(HIDDEN) {imm_vec_ref,                rep_spec::any};

  inline constexpr const val_spec val_imm J_A(HIDDEN) {taggable_imm_type,          rep_spec::any};
  inline constexpr const val_spec val_imm_tag J_A(HIDDEN) {taggable_imm_type,      rep_spec::tagged};
}
