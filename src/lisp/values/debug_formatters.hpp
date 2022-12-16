#pragma once

#include "strings/formatting/formatter.hpp"
#include "lisp/values/lisp_imm.hpp"

namespace j::lisp::inline values {
  struct lisp_object;

  extern const strings::formatter_known_length<
    lisp_object,
    lisp_str,
    lisp_vec
  > & g_lisp_object_formatter;

  extern const strings::formatter_known_length<
    lisp_imm,
    lisp_f32, lisp_i64,
    lisp_nil, lisp_bool,
    lisp_fn_ref, lisp_sym_id,
    lisp_str_ref, lisp_vec_ref
  > & g_lisp_imm_formatter;
}
