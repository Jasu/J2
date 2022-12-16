#pragma once

#include "lisp/lisp_fwd.hpp"

namespace j::lisp::reader {
  struct token_iterator;
  struct state;

  lisp_imm build_form_ast(reader::state & s, token_iterator it);

  lisp_vec * build_ast(reader::state & s);
}
