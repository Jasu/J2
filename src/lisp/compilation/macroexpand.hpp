#pragma once

#include "lisp/values/lisp_imm.hpp"
#include "lisp/common/id.hpp"

namespace j::lisp::symbols {
  struct symbol;
}

namespace j::lisp::compilation {
  struct source_compilation_context;

  enum class macroexpand_status : u8_t{
    not_macro,
    did_expand,
    pending,
  };

  struct macroexpand_result final {
    macroexpand_status status;
    lisp_imm result;
    id pending_symbol;
  };

  [[nodiscard]] macroexpand_result macroexpand_step(source_compilation_context & ctx, lisp_vec_ref vec);

  [[nodiscard]] macroexpand_result macroexpand_step(source_compilation_context & ctx, id head, imms_t body);

  [[nodiscard]] macroexpand_result macroexpand_step(symbols::symbol * J_NOT_NULL head, imms_t body);
}
