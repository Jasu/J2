#pragma once

#include "lisp/air/passes/air_pass.hpp"

namespace j::lisp::air::inline passes {
  extern const air_pass act_rec_compute_pass;
  extern const air_pass act_rec_convert_pass;
  extern const air_pass basic_validation_pass;
  extern const air_pass convert_reps_pass;
  extern const air_pass count_exprs_pass;
  extern const air_pass dce_pass;
  extern const air_pass lex_vars_pass;
  extern const air_pass propagate_barriers_pass;
  extern const air_pass propagate_reps_pass;
  extern const air_pass propagate_types_pass;
  extern const air_pass reaching_defs_pass;
  extern const air_pass reassociate_pass;
  extern const air_pass simplify_pass;
  extern const air_pass trs_simplify_pass;
}
