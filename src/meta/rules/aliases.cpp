#include "meta/rules/aliases.hpp"
#include "strings/string_map.hpp"
#include "meta/rules/term_expr.hpp"
#include "meta/rules/test_case.hpp"

J_DEFINE_EXTERN_STRING_MAP(j::meta::rules::alias);

namespace j::meta::inline rules {
  J_A(NODISC,RNN) term_expr * alias::get_term_expr() const noexcept {
    J_ASSERT(type == alias_type::macro);
    return copy(alias_macro.te);
  }

  J_A(NODISC,RNN) matcher_base * alias::get_matcher() const noexcept {
    J_ASSERT(type == alias_type::macro);
    return alias_macro.matcher->clone();
  }

  void alias_table::add_alias(strings::const_string_view name, term * J_NOT_NULL t) {
    J_ASSERT_NOT_NULL(name);
    if (!aliases.emplace(name, t).second) {
      J_THROW("Duplicate alias {}", name);
    }
  }

  void alias_table::add_alias(strings::const_string_view name, term_expr * J_NOT_NULL te, matcher_base * J_NOT_NULL m) {
    J_ASSERT_NOT_NULL(name);
    if (!aliases.emplace(name, te, m).second) {
      J_THROW("Duplicate macro {}", name);
    }
  }
  J_A(NODISC) alias * alias_table::maybe_at(strings::const_string_view name) {
    J_ASSERT_NOT_NULL(name);
    alias * result = aliases.maybe_at(name);
    return result ? result : parent ? parent->maybe_at(name) : nullptr;
  }

  J_A(RNN,NODISC) alias_table * alias_table::create(alias_table * parent) noexcept {
    return ::new alias_table(parent);}

  alias_table * alias_table::destroy(alias_table * J_NOT_NULL table) noexcept {
    alias_table * result = table->parent;
    ::delete table;
    return result;
  }

  alias_table::alias_table(alias_table * parent) noexcept
    : parent(parent)
  { }
}
