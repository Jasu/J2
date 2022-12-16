#pragma once

#include "lisp/air/exprs/expr_path.hpp"
#include "lisp/air/exprs/expr_builder.hpp"

namespace j::lisp::air::exprs {
  enum class expr_source_type {
    none = 0U,

    path,
    const_val,
    sym_val,
  };

  struct expr_source final {
    expr_source_type type = expr_source_type::none;

    union {
      expr_path path;
      lisp_imm const_val;
      id sym_val;
    };

    inline expr_source() noexcept
    : path{} {}

    inline expr_source(const expr_path & path) noexcept
      : type(expr_source_type::path),
        path(path)
    { }

    inline expr_source(const lisp_imm & const_val) noexcept
      : type(expr_source_type::const_val),
        const_val(const_val)
    { }

    inline expr_source(id id) noexcept
      : type(expr_source_type::sym_val),
        sym_val(id)
    { }

    template<typename Expr>
    Expr * get(expr_builder & builder, Expr * J_NOT_NULL cur) const noexcept {
      switch (type) {
      case expr_source_type::none:
        return nullptr;

      case expr_source_type::path:
        return path.traverse(cur);
      case expr_source_type::const_val:
        return builder.create(expr_type::ld_const, const_val);
      case expr_source_type::sym_val:
        return builder.create(expr_type::sym_val_rd, sym_val);
      }
    }
  };

}
