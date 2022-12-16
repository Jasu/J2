#include "lisp/packages/parse_tlf.hpp"

#include "lisp/compilation/macroexpand.hpp"
#include "lisp/env/static_ids.hpp"
#include "lisp/values/lisp_imms.hpp"

namespace j::lisp::packages {
  [[nodiscard]] tlf_parse_result parse_tlf(compilation::source_compilation_context & ctx, lisp_imm ast) {
    if (!ast.is_vec_ref()) {
      return {tlf_type::load_time_expr, {}, ast};
    }

    lisp_vec_ref vec = ast.as_vec_ref();
    imms_t body = vec->value();
    if (body.empty()) {
      J_THROW("Empty vec as a top-level form.");
    }


    id name = body.pop_front().as_sym_id();
    J_ASSERT(name);
    namespace gid = env::global_static_ids;
    if (name == gid::id_defun) {
      return {tlf_type::fn, {}, ast};
    } else if (name == gid::id_defmacro) {
      return {tlf_type::macro, {}, ast};
    } else if (name == gid::id_def) {
      return {tlf_type::var, {}, ast};
    } else if (name == gid::id_const) {
      return {tlf_type::constant, {}, ast};
    } else if (name == gid::id_import) {
      return {tlf_type::import, {}, ast};
    }

    if (name.package_id() < package_id_min_user) {
      return {tlf_type::load_time_expr, {}, ast};
    }

    auto macroexpand_result = compilation::macroexpand_step(ctx, name, body);
    switch (macroexpand_result.status) {
    case compilation::macroexpand_status::not_macro:
      return {tlf_type::load_time_expr, {}, ast};
    case compilation::macroexpand_status::pending:
      return {tlf_type::pending, name, ast};
    case compilation::macroexpand_status::did_expand:
      return parse_tlf(ctx, macroexpand_result.result);
    }
  }
}
