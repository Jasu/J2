#include "lisp/env/context.hpp"
#include "lisp/packages/pkg.hpp"

namespace j::lisp::env {
  J_A(NODESTROY,ND) thread_local j::util::context_stack<env_ctx> env_context{env_ctx{}};

  env_ctx::env_ctx(packages::pkg * J_NOT_NULL pkg) noexcept
    : env(pkg->env),
      current_pkg(pkg)
  { }
}
