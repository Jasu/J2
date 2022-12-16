#pragma once

#include "files/paths/path.hpp"

namespace j::lisp::cir::inline ssa {
  struct cir_ssa;
}

namespace j::lisp::cir::inline debug {
  void live_range_dump(const files::path & path, const cir_ssa & ssa);
}
