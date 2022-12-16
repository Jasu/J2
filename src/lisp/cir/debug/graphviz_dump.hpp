#pragma once

#include "files/paths/path.hpp"
#include "lisp/cir/ssa/cir_ssa.hpp"

namespace j::lisp::cir::inline debug {
  void graphviz_dump(const files::path & path, const cir_ssa & ssa);
}
