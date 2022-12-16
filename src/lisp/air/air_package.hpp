#pragma once

#include "lisp/env/static_pkg_def.hpp"

namespace j::lisp::env {
  struct environment;
}
namespace j::lisp::inline values {
  struct lisp_str;
  struct lisp_vec_ref;
}
namespace j::lisp::packages {
  struct pkg;
}

#define J_STATIC_IDS_AIR(M) M(heap, "*heap*")M(empty_vec, "*empty-vec*")M(alloc)M(debug)

namespace j::lisp::air {
  extern const lisp_vec_ref g_empty_vec_ref;

  J_SPD_DECLARE_STATIC_PKG(static_ids, 3, AIR)

  J_RETURNS_NONNULL packages::pkg * create_air_package(env::environment * J_NOT_NULL env);
}
