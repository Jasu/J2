#pragma once

#include "strings/string_map_fwd.hpp"

namespace j::meta {
  struct term;
}

namespace j::meta::inline rules {
  struct term_expr;
  struct matcher_base;
  enum class alias_type : u8_t {
    none,
    term,
    macro,
  };

  struct alias {
    J_A(AI,ND) inline alias() noexcept : type(alias_type::none), alias_term(nullptr) { }
    J_A(AI,ND) explicit inline alias(term * J_NOT_NULL t) noexcept : type(alias_type::term), alias_term(t) { }
    J_A(AI,ND) inline alias(term_expr * J_NOT_NULL te, matcher_base * J_NOT_NULL matcher) noexcept : type(alias_type::macro), alias_macro{te, matcher} { }

    J_A(AI,ND) inline alias(const alias & rhs) noexcept : type(rhs.type), alias_macro(rhs.alias_macro) { }
    J_A(AI,ND) inline alias & operator=(const alias & rhs) noexcept {
      type = rhs.type;
      alias_macro = rhs.alias_macro;
      return *this;
    }

    alias_type type;
    union {
      term * alias_term;
      struct {
        term_expr * te;
        matcher_base * matcher;
      } alias_macro;
    };

    J_A(NODISC,RNN,AI,ND) inline term * get_term() const noexcept {
      J_ASSERT(type == alias_type::term);
      return alias_term;
    }

    J_A(NODISC,RNN) term_expr * get_term_expr() const noexcept;
    J_A(NODISC,RNN) matcher_base * get_matcher() const noexcept;
  };
}

J_DECLARE_EXTERN_STRING_MAP(j::meta::rules::alias);

namespace j::meta::inline rules {
  struct alias_table {
    J_BOILERPLATE(alias_table, CTOR_DEL, COPY_DEL)

    void add_alias(strings::const_string_view name, term * J_NOT_NULL t);
    void add_alias(strings::const_string_view name, term_expr * J_NOT_NULL te, matcher_base * J_NOT_NULL m);
    J_A(NODISC) alias * maybe_at(strings::const_string_view name);

    J_A(RNN,NODISC) static alias_table * create(alias_table * parent) noexcept;
    static alias_table * destroy(alias_table * J_NOT_NULL parent) noexcept;
  private:
    explicit alias_table(alias_table * parent) noexcept;
    strings::string_map<alias> aliases;
    alias_table * parent = nullptr;
  };
}
