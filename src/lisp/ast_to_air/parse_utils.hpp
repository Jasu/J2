#pragma once

#include "lisp/values/lisp_imms.hpp"
#include "lisp/env/static_ids.hpp"

namespace j::lisp::ast_to_air {
  imm_type_mask parse_type_mask(imms_t body) {
    using gid = env::global_static_ids::id_index;
    J_ASSERT(body.size());
    imm_type_mask result;
    for (auto & p : body) {
      auto sym = p.as_sym_id().value();
      J_ASSERT(sym.package_id() == package_id_global);
      switch (sym.index()) {
      case (u32_t)gid::idx_null:
        result.set(imm_nil);
        break;
      case (u32_t)gid::idx_bool:
        result.set(imm_bool);
        break;
      case (u32_t)gid::idx_int:
        result.set(imm_i64);
        break;
      case (u32_t)gid::idx_float:
        result.set(imm_f32);
        break;
      case (u32_t)gid::idx_symbol:
        result.set(imm_sym_id);
        break;
      case (u32_t)gid::idx_vec:
        result.set(imm_vec_ref);
        break;
      case (u32_t)gid::idx_string:
        result.set(imm_str_ref);
        break;
      case (u32_t)gid::idx_function:
        result.set(imm_fn_ref);
        break;
      case (u32_t)gid::idx_rec:
        result.set(imm_rec_ref);
        break;
      default:
        J_THROW("Unknown type");
      }
    }
    J_ASSERT(result, "Empty type mask");
    return result;
  }

  inline lisp_imm * value_if_quoted(lisp_imm & form) {
    if (!form.is_vec_ref()) {
      return nullptr;
    }
    auto vec_body = form.as_vec_ref()->value();
    if (!vec_body || !vec_body.front().is_sym_id() || vec_body.front().as_sym_id().value() != env::global_static_ids::id_quote) {
      return nullptr;
    }

    J_ASSERT(vec_body.size() == 2);
    return &vec_body[1];
  }
}
