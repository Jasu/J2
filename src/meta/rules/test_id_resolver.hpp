#pragma once

#include "containers/pair.hpp"
#include "strings/string.hpp"
#include "containers/vector.hpp"
#include "meta/rules/id_resolver.hpp"

namespace j::lisp::env {
  struct environment;
}

namespace j::meta::inline rules {
  struct test_id_resolver final : public id_resolver {
    explicit test_id_resolver(lisp::env::environment * J_NOT_NULL env) noexcept;
    J_A(NODISC) lisp::id resolve(strings::const_string_view pkg, strings::const_string_view name) final;
    J_A(NODISC) lisp::id maybe_resolve(strings::const_string_view pkg, strings::const_string_view name) final;
    J_A(NODISC) bool has_id(strings::const_string_view pkg, strings::const_string_view name) noexcept final;

    vector<strings::string> created_packages;
    vector<pair<strings::string, strings::string>> created_ids;
  private:
    [[no_unique_address]] static_id_resolver static_resolver;
    lisp::env::environment * test_env;
  };
}
