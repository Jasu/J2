#include "lisp/compilation/compile_symbol.hpp"
#include "lisp/compilation/context.hpp"
#include "lisp/packages/pkg.hpp"
#include "lisp/compilation/compile_var.hpp"
#include "lisp/compilation/compile_defun.hpp"

namespace j::lisp::compilation {
  void compile_symbol(packages::pkg & pkg, symbols::symbol & sym) {
    auto & pkgc = pkg.get_pkg_context();
    context ctx(pkgc, sym);
    if (sym.value_info.is_function() || sym.value_info.is_macro()) {
      compile_defun(ctx);
    } else {
      compile_var(ctx);
    }
  }
}
