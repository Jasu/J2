#include "meta/rules/id_resolver.hpp"
#include "lisp/env/static_ids.hpp"
#include "exceptions/exceptions.hpp"

namespace j::meta::inline rules {
  J_A(NODISC) lisp::id static_id_resolver::resolve(strings::const_string_view pkg, strings::const_string_view name) {
    return lisp::env::get_static_id(pkg, name);
  }
  J_A(NODISC) lisp::id static_id_resolver::maybe_resolve(strings::const_string_view pkg, strings::const_string_view name) {
    return lisp::env::maybe_get_static_id(pkg, name);
  }
  J_A(NODISC) bool static_id_resolver::has_id(strings::const_string_view pkg, strings::const_string_view name) noexcept {
    return lisp::env::is_static_id(pkg, name);
  }

  J_A(NODISC) lisp::id null_id_resolver::resolve(strings::const_string_view pkg, strings::const_string_view name) {
    J_THROW("Null resolver: Tried to look up {}:{}", pkg, name);
  }

  J_A(NODISC) lisp::id null_id_resolver::maybe_resolve(strings::const_string_view, strings::const_string_view) {
    return lisp::id(nullptr);
  }
  J_A(NODISC) bool null_id_resolver::has_id(strings::const_string_view, strings::const_string_view) noexcept {
    return false;
  }
}
