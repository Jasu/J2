#pragma once

#include "lisp/values/lisp_imm.hpp"

namespace j::lisp::functions {
  struct func_info;
}
namespace j::lisp::air::inline functions {
  struct air_function;
}
namespace j::lisp::compilation {
  struct context;

  struct prototype_init final {
    id fn_name{};
    u32_t num_optional:30 = 0U;
    bool has_rest:1 = false;
    span<id> params{};
  };

  functions::func_info * parse_func_info(context & ctx, imms_t param_list);

  void compile_defun(context & ctx);
  void compile_defun(context & ctx, imms_t param_list, imms_t vec_body);
  void compile_defun(context & ctx, const prototype_init & proto, air::air_function * J_NOT_NULL air_fn);
}
