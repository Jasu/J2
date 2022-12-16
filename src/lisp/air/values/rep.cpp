#include "lisp/air/values/rep.hpp"

namespace j::lisp::air::inline values {
  J_AV(HIDDEN) constinit const rep_spec possible_reps[num_imm_types]{
    [imm_i64]         = rep_spec::tagged_or_untagged,
    [imm_f32]         = rep_spec::tagged_or_untagged,
    [imm_bool]        = rep_spec::tagged,
    [imm_nil]         = rep_spec::tagged,
    [imm_sym_id]      = rep_spec::tagged,
    [imm_fn_ref]      = rep_spec::tagged_or_untagged,
    [imm_closure_ref] = rep_spec::tagged_or_untagged,
    [imm_str_ref]     = rep_spec::tagged_or_untagged,
    [imm_vec_ref]     = rep_spec::any,
    [imm_rec_ref]     = rep_spec::tagged_or_untagged,
    [imm_act_rec]     = rep_spec::untagged,
    [imm_range]       = rep_spec::range,
  };

  J_A(NODISC,CONST) rep_spec get_possible_reps(imm_type_mask mask) noexcept {
    if (mask.has_single_value()) {
      return possible_reps[mask.first_value()];
    }
    return mask == range_imm_type ? rep_spec::range_or_tagged : rep_spec::tagged;
  }
}
