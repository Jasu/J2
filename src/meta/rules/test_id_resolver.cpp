#include "meta/rules/test_id_resolver.hpp"
#include "lisp/env/environment.hpp"
#include "lisp/env/context.hpp"
#include "lisp/packages/pkg.hpp"

namespace j::meta::inline rules {
  test_id_resolver::test_id_resolver(lisp::env::environment * J_NOT_NULL env) noexcept
    : test_env(env)
  {
  }

  J_A(NODISC) lisp::id test_id_resolver::resolve(strings::const_string_view pkg, strings::const_string_view name) {
    return maybe_resolve(pkg, name);
  }

J_A(NODISC) lisp::id test_id_resolver::maybe_resolve(strings::const_string_view pkg, strings::const_string_view name) {
    lisp::id result = static_resolver.maybe_resolve(pkg, name);
    if (result) {
      return result;
    }
    lisp::packages::pkg * p = test_env->try_get_package(pkg);
    if (!p) {
      p = test_env->new_package(pkg);
      created_packages.push_back(pkg);
    }
    result = p->try_get_id_of(name);
    if (result) {
      return result;
    }
    created_ids.push_back(pair<strings::string, strings::string>{pkg, name});
    return p->emplace_id(name);
  }

  J_A(NODISC) bool test_id_resolver::has_id(strings::const_string_view, strings::const_string_view) noexcept {
    return true;
  }

}
