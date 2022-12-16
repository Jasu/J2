#pragma once

#include <detail/preamble.hpp>

#include "mem/shared_ptr.hpp"
#include "lisp/values/lisp_imms.hpp"
#include "lisp/values/lisp_str.hpp"

namespace j::lisp::env {
  struct environment;
}

#define EQ_CASE(CODE, VAL) SUBCASE(CODE) {                          \
  REQUIRE_UNARY(run_code(e, "(defun f () " CODE ")", "f").eq(VAL)); \
}
#define EQ_CASE_1(CODE, A, VAL) SUBCASE(CODE) {                        \
  REQUIRE_UNARY(run_code(e, "(defun f (a) " CODE ")", "f", A).eq(VAL)); \
}
#define EQ_CASE_2(CODE, A, B, VAL) SUBCASE(CODE) {                          \
  REQUIRE_UNARY(run_code(e, "(defun f (a b) " CODE ")", "f", A, B).eq(VAL)); \
}

namespace j::lisp::tests {
  j::mem::shared_ptr<env::environment> make_env();

  lisp_imm run_code(const j::mem::shared_ptr<env::environment> & e, const char * J_NOT_NULL code, const char * J_NOT_NULL fn, imms_t args);

  lisp_imm run_code(const j::mem::shared_ptr<env::environment> & e, const char * J_NOT_NULL code, const char * J_NOT_NULL fn);

  inline lisp_imm run_code(const char * J_NOT_NULL code, const char * J_NOT_NULL fn) {
    return run_code(make_env(), code, fn);
  }

  inline lisp_imm run_code(const char * J_NOT_NULL code, const char * J_NOT_NULL fn, imms_t args) {
    return run_code(make_env(), code, fn, args);
  }

  template<typename... Args>
  inline lisp_imm run_code(const char * J_NOT_NULL code, const char * J_NOT_NULL fn, lisp_imm arg, Args... args) {
    lisp_imm imms[]{arg, args...};
    return run_code(code, fn, imms);
  }

  template<typename... Args>
  inline lisp_imm run_code(const j::mem::shared_ptr<env::environment> & e, const char * J_NOT_NULL code, const char * J_NOT_NULL fn, lisp_imm arg, Args... args) {
    lisp_imm imms[]{arg, args...};
    return run_code(e, code, fn, imms);
  }
}
