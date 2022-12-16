#include "lisp/packages/parse_import.hpp"

#include "lisp/packages/pkg.hpp"
#include "lisp/values/lisp_imms.hpp"
#include "lisp/env/env_compiler.hpp"

namespace j::lisp::packages {
  void parse_import(pkg * J_NOT_NULL importer, lisp_imm form, env::env_compiler * J_NOT_NULL env_compiler) {
    auto vec = form.as_vec_ref()->value();
    J_ASSERT(vec.size() == 2U, "Todo more import forms");
    auto head = vec[1];
    id alias, name;
    if (head.is_sym_id()) {
      // (import %air) ;; The name of the package will be available locally
      alias = name = head.as_sym_id();
    } else if (head.is_vec_ref()) {
      vec = head.as_vec_ref()->value();
      // (import (%air foo)) ;; Alias %air to foo
      J_REQUIRE(vec.size() == 2, "Import entries must be pairs (package-name alias)");
      name = vec[0].as_sym_id();
      alias = vec[1].as_sym_id();
    } else {
      J_THROW("TODO more import forms");
    }

    importer->symbol_table.package_imports.add_package_import(
      alias,
      env_compiler->compile_pkg(importer->name_of(name)));
    // env_compiler->pending.notify_available(alias);
  }
}
