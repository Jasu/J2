#include "lisp/compilation/macroexpand.hpp"
#include "lisp/compilation/source_compilation.hpp"
#include "lisp/values/lisp_imms.hpp"
#include "lisp/symbols/symbol.hpp"

namespace j::lisp::compilation {
  namespace {
    const macroexpand_result not_macro{
      macroexpand_status::not_macro,
      {},
      id::none,
    };

    macroexpand_result pending(id id) {
      return {
        macroexpand_status::pending,
        {},
        id,
      };
    }
  }

  [[nodiscard]] macroexpand_result macroexpand_step(source_compilation_context & ctx, lisp_vec_ref vec) {
    imms_t body = vec->value();
    if (body.empty() || !body.front().is_sym_id() ) {
      return not_macro;
    }
    const id name = body.pop_front().as_sym_id();
    return macroexpand_step(ctx, name, body);
  }

  [[nodiscard]] macroexpand_result macroexpand_step(source_compilation_context & ctx, id name, imms_t body) {
    if (!name) {
      return not_macro;
    }
    symbols::symbol * const sym = ctx.get_symbol(name, compilation::optional_func);
    return sym ? macroexpand_step(sym, body) : pending(name);
  }

  [[nodiscard]] macroexpand_result macroexpand_step(symbols::symbol * J_NOT_NULL head, imms_t body) {
    if (!head->value_info.is_macro()) {
      return not_macro;
    }
    J_REQUIRE(!head->is_initializing(), "Macro is recursive.");

    lisp_fn_ptr_t fn = head->value.as_fn_ref().value();
    J_ASSUME_NOT_NULL(fn);
    return {
      macroexpand_status::did_expand,
      lisp_imm{fn(reinterpret_cast<u64_t*>(body.begin()), body.size())},
      {}
    };
  }
}
