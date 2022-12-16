#pragma once

#include "lisp/values/lisp_imm.hpp"

namespace j::lisp::env {
  struct env_compiler;
}

namespace j::lisp::packages {
  struct pkg;

  void parse_import(pkg * J_NOT_NULL importer, lisp_imm form, env::env_compiler * J_NOT_NULL env_compiler);
}
