#pragma once

#include "lisp/common/id.hpp"

namespace j::meta::inline rules {
  /// Resolve ids known at meta-compile-time, i.e. known static id:s or test ids.
  struct id_resolver {
    J_A(NODISC) virtual lisp::id resolve(strings::const_string_view pkg, strings::const_string_view name) = 0;
    J_A(NODISC) virtual lisp::id maybe_resolve(strings::const_string_view pkg, strings::const_string_view name) = 0;
    J_A(NODISC) virtual bool has_id(strings::const_string_view pkg, strings::const_string_view name) noexcept = 0;
  };

  struct static_id_resolver final : public id_resolver {
    J_A(NODISC) lisp::id resolve(strings::const_string_view pkg, strings::const_string_view name) final;
    J_A(NODISC) lisp::id maybe_resolve(strings::const_string_view pkg, strings::const_string_view name) final;
    J_A(NODISC) bool has_id(strings::const_string_view pkg, strings::const_string_view name) noexcept final;
  };

  struct null_id_resolver final : public id_resolver {
    J_A(NODISC) lisp::id resolve(strings::const_string_view pkg, strings::const_string_view name) final;
    J_A(NODISC) lisp::id maybe_resolve(strings::const_string_view pkg, strings::const_string_view name) final;
    J_A(NODISC) bool has_id(strings::const_string_view pkg, strings::const_string_view name) noexcept final;
  };
}
