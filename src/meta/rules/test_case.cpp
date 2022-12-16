#include "meta/rules/test_case.hpp"
#include "containers/trivial_array.hpp"
#include "lisp/packages/pkg.hpp"
#include "meta/rules/test_id_resolver.hpp"
#include "meta/module.hpp"
#include "strings/format.hpp"
#include "meta/term.hpp"

namespace j::meta::inline rules {

  namespace s = strings;
  void register_test_case_types(module * J_NOT_NULL mod) {
    trs_config & conf = mod->trs_config;

    conf.test_case_matcher_type = make_enum("MatcherType", {"MatchTerm", "MatchTerms", "ValEq", "ValEqId"});
    mod->define_node(conf.test_case_matcher_type);

    conf.test_suite = ::new struct_def("TestSuite", (const char*[]){"Name", "Tests"});
    mod->define_node(conf.test_suite);

    conf.test_case = ::new struct_def("TestCase", (const char*[]){"Name", "Input", "Match"});
    mod->define_node(conf.test_case);

    conf.test_case_matcher = ::new struct_def("Matcher", (const char*[]){
        "Type",
        "Path",
        "Target",
        "ValueType",
        "Expect",
      });
    mod->define_node(conf.test_case_matcher);

    conf.test_case_match_term = ::new struct_def("MatchTerm", (const char*[]){"Term", "Children"});
    mod->define_node(conf.test_case_match_term);

    conf.test_case_match_val_eq = ::new struct_def("MatchValEq", (const char*[]){"Value", "IsCompiled"});
    mod->define_node(conf.test_case_match_val_eq);
  }

  namespace {
    attr_value make_matcher_struct(module & mod,
                                   s::string && path,
                                   attr_value && target,
                                   matcher_base * J_NOT_NULL matcher)
    {
      return attr_value(mod.trs_config.test_case_matcher, (pair<const char *, attr_value>[]){
        {"Type", attr_value(mod.trs_config.test_case_matcher_type, (u64_t)matcher->get_type())},
        {"Path", attr_value(path)},
        {"Target", static_cast<attr_value &&>(target)},
        {"ValueType", attr_value((node*)matcher->get_val_type(mod))},
        {"Expect", matcher->to_meta_struct(mod, path)},
      });
    }
  }

  matcher_base::~matcher_base() {
  }

  term_matcher::term_matcher(const term * J_NOT_NULL expected, span<matcher_base*> matchers)
    : expected(expected)
  {
    J_ASSERT(matchers.size() <= expected->num_inputs + expected->num_consts + expected->num_data);
    j::memcpy(this->matchers, matchers.begin(), J_PTR_SZ * matchers.size());
  }

  J_A(NODISC) matcher_type term_matcher::get_type() const noexcept {
    return matcher_type::match_term;
  }

  J_A(NODISC,RNN) val_type * term_matcher::get_val_type(module & mod) const noexcept {
    return mod.trs_term_type;
  }

  attr_value term_matcher::to_meta_struct(module & mod, const strings::string & path) {
    attr_value children[6];
    i32_t i = 0;
    for (i32_t j = 0; j < 6; ++j) {
      if (!matchers[j]) {
        continue;
      }
      bool strip_root = (matchers[j]->get_type() == matcher_type::match_term || matchers[j]->get_type() == matcher_type::match_terms) && path == "root";
      children[i++] = make_matcher_struct(mod,
                                          strip_root ? expected->ctor[j]->name : path + "." + expected->ctor[j]->name,
                                          attr_value((node*)expected->ctor[j]),
                                          matchers[j]);
    }
    return attr_value(mod.trs_config.test_case_match_term,
                      (pair<const char*, attr_value>[]){
                        {"Term", attr_value((node*)expected)},
                        {"Children", attr_value(span<attr_value>(children, i))},
                      });
  }
  J_A(NODISC,RNN) matcher_base * term_matcher::clone() const noexcept {
    i32_t sz = expected->num_inputs + expected->num_consts + expected->num_data;
    matcher_base * children[sz];
    for (i32_t i = 0; i < sz; ++i) {
      children[i] = matchers[i] ? matchers[i]->clone() : nullptr;
    }
    return ::new term_matcher(expected, span(children, sz));
  }

  term_matcher::~term_matcher() {
    for (i32_t i = 0; i < 6; ++i) {
      ::delete matchers[i];
    }
  }

  terms_matcher::terms_matcher(span<term_matcher*> && matchers, val_type * J_NOT_NULL type) noexcept
    : matchers(containers::move, matchers),
      type(type)
  {
    J_ASSERT(type->is_trs_terms_type);
  }

  J_A(NODISC) matcher_type terms_matcher::get_type() const noexcept {
    return matcher_type::match_terms;
  }

  J_A(NODISC,RNN) val_type * terms_matcher::get_val_type(module &) const noexcept {
    return type;
  }

  attr_value terms_matcher::to_meta_struct(module & mod, const strings::string & path) {
    i32_t sz = matchers.size();
    attr_value terms[sz];
    for (i32_t i = 0; i < sz; ++i) {
      if (matchers[i]) {
        terms[i] = make_matcher_struct(
          mod,
          s::format("{}[{}]", path, i),
          attr_value(i),
          matchers[i]);
      }
    }
    return attr_value(span<attr_value>(terms, sz));
  }

  J_A(NODISC,RNN) matcher_base * terms_matcher::clone() const noexcept {
    i32_t sz = matchers.size();
    term_matcher* children[sz];
    for (i32_t i = 0; i < sz; ++i) {
      children[i] = matchers[i] ? (term_matcher*)matchers[i]->clone() : nullptr;
    }
    return ::new terms_matcher(span(children, sz), type);
  }

  terms_matcher::~terms_matcher() {
    for (matcher_base * m : matchers) {
      ::delete m;
    }
  }
  val_eq_matcher::val_eq_matcher(attr_value && value, val_type * J_NOT_NULL type) noexcept
    : value(static_cast<attr_value &&>(value)),
      type(type)
  {
  }

  val_eq_matcher::val_eq_matcher(strings::const_string_view value, val_type * J_NOT_NULL type) noexcept
    : value(value),
      type(type),
      is_rhs_val(true)
  {
  }

  J_A(NODISC) matcher_type val_eq_matcher::get_type() const noexcept {
    return matcher_type::val_eq;
  }

  J_A(NODISC,RNN) val_type * val_eq_matcher::get_val_type(module &) const noexcept {
    return type;
  }

  attr_value val_eq_matcher::to_meta_struct(module & mod, const strings::string &) {
    if (value.is_id()) {
      value.id.resolved = mod.test_resolver->resolve(value.id.name.pkg, value.id.name.name);
    }
    return attr_value(mod.trs_config.test_case_match_val_eq, (pair<const char *, attr_value>[]){
      {"Value", attr_value(value)},
      {"IsCompiled", attr_value(is_rhs_val)},
    });
  }

  J_A(NODISC,RNN) matcher_base * val_eq_matcher::clone() const noexcept {
    return ::new val_eq_matcher(value, type, is_rhs_val);
  }

  val_eq_matcher::val_eq_matcher(const attr_value & value, val_type * J_NOT_NULL type, bool is_rhs_val) noexcept
    : value(value),
      type(type),
      is_rhs_val(is_rhs_val)
  {
  }

  test_case::test_case(s::const_string_view name) noexcept
    : name(name)
  {
  }

  test_case::~test_case() {
    ::delete construct;
    ::delete matcher;
    construct = nullptr;
    matcher = nullptr;
  }

  test_suite::test_suite(strings::string && name) noexcept
    : name(static_cast<strings::string &&>(name))
  {
  }

  test_suite::~test_suite() {
    for (auto t : tests) {
      ::delete t;
    }
  }

  void test_suite::add_test(test_case* J_NOT_NULL test) {
    tests.push_back(test);
  }

  J_A(NODISC) test_data get_test_data(module * J_NOT_NULL mod) {
    trivial_array_copyable<attr_value> test_suites(containers::uninitialized, mod->test_suites.size());
    expr_scope_root root_scope = mod->make_root_scope();
    s::string root = "root";
    for (const test_suite * ts : mod->test_suites) {
      trivial_array_copyable<attr_value> test_cases(containers::uninitialized, ts->tests.size());
      for (const test_case * tc : ts->tests) {
        J_ASSERT_NOT_NULL(tc, tc->name, tc->construct, tc->matcher);
        tc->construct->resolve(root_scope, mod->test_resolver);
        test_cases.initialize_element(mod->trs_config.test_case,
                                      (pair<const char*, attr_value>[]){
                                        {"Name", attr_value(tc->name)},
                                        {"Match", tc->matcher->to_meta_struct(*mod, root)},
                                        {"Input", tc->construct->compile(root_scope, mod->trs_term_type)},
                                      });
      }
      test_suites.initialize_element(mod->trs_config.test_suite,
                                     (pair<const char*, attr_value>[]){
                                       {"Name", attr_value(ts->name)},
                                       {"Tests", attr_value(static_cast<trivial_array_copyable<attr_value> &&>(test_cases))},
                                     });
    }

    trivial_array_copyable<attr_value> created_pkgs(containers::uninitialized, mod->test_resolver->created_packages.size());
    trivial_array_copyable<attr_value> created_ids(containers::uninitialized, mod->test_resolver->created_ids.size());
    for (const auto & pkg : mod->test_resolver->created_packages) {
      created_pkgs.initialize_element(pkg);
    }
    for (const auto & p : mod->test_resolver->created_ids) {
      trivial_array_copyable<attr_value> tuple(containers::uninitialized, 2);
      tuple.initialize_element(p.first);
      tuple.initialize_element(p.second);
      created_ids.initialize_element(static_cast<trivial_array_copyable<attr_value> &&>(tuple));
    }
    return {
      attr_value(static_cast<trivial_array_copyable<attr_value> &&>(test_suites)),
      attr_value(static_cast<trivial_array_copyable<attr_value> &&>(created_pkgs)),
      attr_value(static_cast<trivial_array_copyable<attr_value> &&>(created_ids)),
    };
  }
}
