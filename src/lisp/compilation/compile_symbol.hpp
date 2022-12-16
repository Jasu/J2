#pragma once

namespace j::lisp::packages {
  struct pkg;
}
namespace j::lisp::symbols {
  struct symbol;
}

namespace j::lisp::compilation {
  void compile_symbol(packages::pkg & pkg, symbols::symbol & sym);
}
