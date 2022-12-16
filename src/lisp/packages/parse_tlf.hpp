#pragma once

#include "lisp/values/lisp_imm.hpp"

namespace j::lisp::compilation {
  struct source_compilation_context;
}

namespace j::lisp::packages {

  enum class tlf_type : u8_t {
    pending,

    import,
    fn,
    macro,
    var,
    constant,
    load_time_expr,
  };

  /// Top-level form's compilation status.
  struct tlf_parse_result final {
    tlf_type type;
    id pending_symbol = id::none;
    lisp_imm ast;
  };

  [[nodiscard]] tlf_parse_result parse_tlf(compilation::source_compilation_context & ctx, lisp_imm ast);
}
