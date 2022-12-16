#pragma once

#include "util/context_stack.hpp"

namespace j::lisp::packages {
  struct pkg;
}
namespace j::lisp::env {
  struct environment;

  struct env_ctx final {
    J_A(AI,ND) inline env_ctx() noexcept = default;

    J_A(AI,ND) inline explicit env_ctx(environment * J_NOT_NULL env) noexcept
      : env(env)
    { }

    explicit env_ctx(packages::pkg * J_NOT_NULL pkg) noexcept;

    environment * env = nullptr;
    packages::pkg * current_pkg = nullptr;
  };

  extern thread_local j::util::context_stack<env_ctx> env_context J_A(ND);

  using env_ctx_guard = j::util::context_stack<env_ctx>::guard;
}
