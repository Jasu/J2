#pragma once

#include "lisp/env/compilation_passes.hpp"
#include "lisp/air/debug/debug_info_map.hpp"

namespace j::lisp::compilation {
  struct context;
}

namespace j::lisp::air::inline functions {
  struct air_function;
}
namespace j::lisp::env {
  struct global_pass_options;
}

namespace j::lisp::air::inline passes {

  void run_air_passes(compilation::context & ctx, j::mem::bump_pool * J_NOT_NULL pool, air::air_function * J_NOT_NULL fn);

  struct air_pass_context final : env::compilation_pass_context_base {
    air_pass_context(compilation::context & ctx,
                     strings::const_string_view dump_path,
                     j::mem::bump_pool * J_NOT_NULL pool, air::air_function * J_NOT_NULL fn,
                     const env::global_pass_options * J_NOT_NULL opts,
                     debug_info_map * dbg) noexcept;

    compilation::context & ctx;
    air::air_function * fn = nullptr;
    debug_info_map * debug_info = nullptr;

    template<typename Name, typename Value>
    J_A(ND) inline void add_debug_info(const exprs::expr * J_NOT_NULL expr, Name && name, Value && value, const char * bg_color = nullptr) {
      if (debug_info) {
        debug_info->add(expr, static_cast<Name &&>(name), static_cast<Value &&>(value), bg_color);
      }
    }

    template<typename Value>
    J_A(ND) inline void add_debug_info_oneline(const exprs::expr * J_NOT_NULL expr, Value && value, const char * bg_color = nullptr) {
      if (debug_info) {
        debug_info->add_oneline(expr, static_cast<Value &&>(value), bg_color);
      }
    }
  };

  struct air_pass final : env::compilation_pass_base {
    void (*execute)(air_pass_context &);

    template<typename... Ts>
    consteval air_pass(void (*J_NOT_NULL execute)(air_pass_context &), Ts... args)
      : compilation_pass_base(args...),
        execute(execute)
    { }
  };
}
