#pragma once

#include "lisp/air/air_package.hpp"

#define J_STATIC_IDS_GLOBAL(M)                                                       \
  M(quote)M(unquote)M(quasiquote)M(pre_ellipsis, "pre-ellipsis")M(post_ellipsis, "post-ellipsis")                 \
  M(defun)M(defmacro)M(def)M(const)M(import)M(progn)M(let)M(call)M(fn) \
  M(plus, "+")M(minus, "-")M(multiply, "*")M(divide, "/")M(modulo, "%") \
  M(and)M(or)M(not)M(eq)M(neq)M(if)M(do_until, "do-until")M(set)M(setp) \
  M(check_type, "check-type")M(int)M(bool)M(float)M(vec)M(rec)M(string)M(function)M(null)M(symbol)    \
  M(make_vec, "make-vec")M(at)M(length)M(copy)M(append)

#define J_STATIC_IDS_KEYWORD(M) M(empty_keyword, ":")

#define J_STATIC_IDS_PACKAGES(M) M(keyword)M(global)M(packages)M(air, "%air")M(const_fold, ":const_fold")

#define J_STATIC_IDS_CONST_FOLD(M)

namespace j::lisp::env {
  J_SPD_DECLARE_STATIC_PKG(keyword_static_ids, 0, KEYWORD)
  J_SPD_DECLARE_STATIC_PKG(global_static_ids, 1, GLOBAL)
  J_SPD_DECLARE_STATIC_PKG(packages_static_ids, 2, PACKAGES)
  J_SPD_DECLARE_STATIC_PKG(const_fold_static_ids, 4, CONST_FOLD)

  inline constexpr i32_t num_static_pkgs = 5;
  extern constinit const packages::static_pkg_defs static_pkgs;

  J_A(NODISC) bool is_static_id(id id) noexcept;
  J_A(NODISC) packages::static_id_name get_static_id_name(id id) noexcept;
  J_A(NODISC) packages::static_id_name maybe_get_static_id_name(id id) noexcept;

  J_A(NODISC) bool is_static_id(strings::const_string_view pkg, strings::const_string_view sym) noexcept;
  J_A(NODISC) id get_static_id(strings::const_string_view pkg, strings::const_string_view sym) noexcept;
  J_A(NODISC) id maybe_get_static_id(strings::const_string_view pkg, strings::const_string_view sym) noexcept;
}
