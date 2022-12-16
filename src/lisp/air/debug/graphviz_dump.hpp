#pragma once

#include "files/paths/path.hpp"
#include "lisp/air/functions/air_function.hpp"

namespace j::lisp::air::inline debug {
  struct debug_info_map;

  void graphviz_dump(const files::path & path,
                     const air_function * J_NOT_NULL fn,
                     const debug_info_map * debug_info = nullptr);
}
