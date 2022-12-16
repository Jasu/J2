#pragma once

#include "containers/vector.hpp"
#include "meta/attr_value.hpp"
#include "lisp/common/id.hpp"

namespace j::meta::inline rules {
  struct id_resolver;
  struct term_expr;

  enum class matcher_type : u8_t {
    match_term,
    match_terms,
    val_eq,
  };

  void register_test_case_types(module * J_NOT_NULL mod);
  struct test_data final {
    attr_value test_suites;
    attr_value created_pkgs;
    attr_value created_ids;
  };

  J_A(NODISC) test_data get_test_data(module * J_NOT_NULL mod);

  struct matcher_base {
    virtual ~matcher_base();
    J_A(NODISC) virtual attr_value to_meta_struct(module & mod, const strings::string & path) = 0;
    J_A(NODISC,RNN) virtual val_type * get_val_type(module & mod) const noexcept = 0;
    J_A(NODISC) virtual matcher_type get_type() const noexcept = 0;
    J_A(NODISC,RNN) virtual matcher_base * clone() const noexcept = 0;
  };

  struct term_matcher final : matcher_base {
    J_BOILERPLATE(term_matcher, COPY_DEL)

    const term * expected;

    term_matcher(const term * J_NOT_NULL expected, span<matcher_base*> matchers);
    J_A(NODISC) attr_value to_meta_struct(module & mod, const strings::string & path);
    J_A(NODISC,RNN) virtual val_type * get_val_type(module & mod) const noexcept;
    J_A(NODISC) matcher_type get_type() const noexcept;
    J_A(NODISC,RNN) matcher_base * clone() const noexcept;

    ~term_matcher();
  private:
    matcher_base * matchers[6] = {nullptr};
  };

  struct terms_matcher final : matcher_base {
    J_BOILERPLATE(terms_matcher, COPY_DEL)

    explicit terms_matcher(span<term_matcher*> && matchers, val_type * J_NOT_NULL type) noexcept;
    J_A(NODISC) attr_value to_meta_struct(module & mod, const strings::string & path);
    J_A(NODISC,RNN) val_type * get_val_type(module & mod) const noexcept;
    J_A(NODISC) matcher_type get_type() const noexcept;
    J_A(NODISC,RNN) matcher_base * clone() const noexcept;

    ~terms_matcher();
  private:
    trivial_array<term_matcher*> matchers;
    val_type * type;
  };

  struct val_eq_matcher final : matcher_base {
    J_BOILERPLATE(val_eq_matcher, COPY_DEL)

    val_eq_matcher(attr_value && value, val_type * J_NOT_NULL type) noexcept;
    val_eq_matcher(strings::const_string_view value, val_type * J_NOT_NULL type) noexcept;
    J_A(NODISC) attr_value to_meta_struct(module & mod, const strings::string & path);
    J_A(NODISC,RNN) val_type * get_val_type(module & mod) const noexcept;
    J_A(NODISC) matcher_type get_type() const noexcept;
    J_A(NODISC,RNN) matcher_base * clone() const noexcept;

  private:
    val_eq_matcher(const attr_value & value, val_type * J_NOT_NULL type, bool is_rhs_val) noexcept;
    attr_value value;
    val_type * type;
    bool is_rhs_val = false;
  };

  struct test_case final {
    J_BOILERPLATE(test_case, COPY_DEL, CTOR_DEL)

    explicit test_case(strings::const_string_view name) noexcept;

    ~test_case();

    strings::string name;
    term_expr * construct = nullptr;
    term_matcher * matcher = nullptr;
  };

  struct test_suite final {
    J_BOILERPLATE(test_suite, COPY_DEL, CTOR_DEL)
    explicit test_suite(strings::string && name) noexcept;
    ~test_suite();
    strings::string name;
    noncopyable_vector<test_case*> tests;

    void add_test(test_case* J_NOT_NULL test);
  };
}
