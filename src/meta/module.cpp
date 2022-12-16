#include "module.hpp"
#include "meta/rules/grammar.hpp"
#include "meta/rules/test_case.hpp"
#include "meta/term.hpp"
#include "meta/rules/test_id_resolver.hpp"
#include "meta/expr.hpp"
#include "meta/fn.hpp"
#include "meta/errors.hpp"
#include "meta/rules/meta_interface.hpp"
#include "strings/format.hpp"
#include "lisp/env/environment.hpp"
#include "exceptions/assert.hpp"
#include "meta/rules/rule.hpp"
#include "meta/rules/nonterminal.hpp"
#include "meta/attr_context.hpp"
#include "meta/basic_node_set.hpp"
#include "meta/gen_file.hpp"
#include "strings/string_map.hpp"

J_DEFINE_EXTERN_STRING_MAP(j::meta::node*);
J_DEFINE_EXTERN_STRING_MAP(j::meta::val_const);
J_DEFINE_EXTERN_STRING_MAP(j::meta::term_stack_info*);

namespace j::meta {
  namespace s = strings;
  val_const::val_const(s::string && value, val_type * J_NOT_NULL type) noexcept
    : value(static_cast<s::string &&>(value)),
      type(type)
  { }

  module::module(const files::path & base_dir) noexcept
    : base_dir(base_dir),
      test_env(::new lisp::env::environment),
      normal_resolver(::new static_id_resolver),
      test_resolver(::new test_id_resolver(test_env)),
      string_sink(j::mem::make_shared<streams::string_sink>()),
      formatted_string_sink(j::mem::static_pointer_cast<streams::sink>(string_sink)),
      string_sink_writer(&formatted_string_sink)
  {
    attr_ctx = &pool.emplace<attr_context>();
    register_parser_structs(this);
    register_test_case_types(this);
    trs_config.define(this);
    (void)define_nonterminal("Any", {});
    (void)define_nonterminal("Anys", {});
  }

  s::string module::expand(const codegen_template & tpl, expr_scope & scope) {
    J_ASSERT(!string_sink->string);
    string_sink_writer.clear_break();
    tpl.expand(string_sink_writer, scope);
    formatted_string_sink.flush();
    return static_cast<s::string &&>(string_sink->string);
  }

  module::~module() {
    ::delete test_resolver;
    ::delete normal_resolver;
    for (auto t : test_suites) {
      ::delete t;
    }
    for (auto s : sections) {
      ::delete s;
    }
  }

  void module::parse_path(const files::path & path) {
    J_ASSERT(path);
    if (path.is_relative()) {
      parse_file(base_dir / path, this);
    } else {
      parse_file(path, this);
    }
  }

  void module::parse_path(strings::const_string_view path) {
    parse_path(files::path(path));
  }

  void module::initialize() noexcept {
    auto s = make_root_scope();
    for (val_type * t : val_types()) {
      J_ASSERT_NOT_NULL(t);
      t->initialize();
    }
    for (term * t : terms()) {
      J_ASSERT_NOT_NULL(t);
      t->initialize(this);
    }
    grammar = make_grammar(*this);
    set_global("Lalr", get_parser_tables(this));
    auto td = get_test_data(this);
    set_global("TestSuites", static_cast<attr_value &&>(td.test_suites));
    set_global("CreatedTestPkgs", static_cast<attr_value &&>(td.created_pkgs));
    set_global("CreatedTestIds", static_cast<attr_value &&>(td.created_ids));
  }

  [[nodiscard]] span<term*> module::terms() noexcept {
    return {reinterpret_cast<term**>(term_ptrs.begin()), (i32_t)term_ptrs.size()};
  }
  [[nodiscard]] span<const term* const> module::terms() const noexcept {
    return {reinterpret_cast<const term* const*>(term_ptrs.begin()), (i32_t)term_ptrs.size()};
  }

  [[nodiscard]] span<nonterminal*> module::nonterminals() noexcept {
    return {reinterpret_cast<nonterminal**>(nonterminal_ptrs.begin()), (i32_t)nonterminal_ptrs.size()};
  }
  [[nodiscard]] span<const nonterminal* const> module::nonterminals() const noexcept {
    return {reinterpret_cast<const nonterminal* const*>(nonterminal_ptrs.begin()), (i32_t)nonterminal_ptrs.size()};
  }

  [[nodiscard]] span<val_type*> module::val_types() noexcept {
    return {reinterpret_cast<val_type**>(value_type_ptrs.begin()), (i32_t)value_type_ptrs.size()};
  }
  [[nodiscard]] span<const val_type* const> module::val_types() const noexcept {
    return {reinterpret_cast<const val_type* const *>(value_type_ptrs.begin()), (i32_t)value_type_ptrs.size()};
  }

  [[nodiscard]] span<const generated_file_set* const> module::gen_file_sets() const noexcept {
    return {reinterpret_cast<const generated_file_set* const *>(gen_file_set_ptrs.begin()), (i32_t)gen_file_set_ptrs.size()};
  }

  [[nodiscard]] span<node* const> module::nodes_by_type(node_type t) noexcept {
    switch (t) {
    case node_term:
      return term_ptrs;
    case node_nonterminal:
      return nonterminal_ptrs;
    case node_value_type:
      return value_type_ptrs;
    default:
      J_THROW("Unsupported node type {}", t);
    }
  }

  [[nodiscard]] span<const node* const> module::nodes_by_type(node_type t) const noexcept {
    return const_cast<module*>(this)->nodes_by_type(t);
  }


  J_A(NODISC,RNN) node * module::node_at(strings::const_string_view name, node_type type) {
    node * n = nodes.at(name);
    J_REQUIRE(n->type & type, "\"{}\" is {}, expected {}.", type);
    return n;
  }

  J_A(NODISC,RNN) const node * module::node_at(strings::const_string_view name, node_type type) const {
    return const_cast<module*>(this)->node_at(name, type);
  }
  J_A(NODISC) node * module::node_maybe_at(strings::const_string_view name, node_type type) noexcept {
    node ** n = nodes.maybe_at(name);
    return (n && ((*n)->type & type)) ? *n : nullptr;
  }

  J_A(NODISC) const node * module::node_maybe_at(strings::const_string_view name, node_type type) const noexcept {
    return const_cast<module*>(this)->node_maybe_at(name, type);
  }

  node * module::define_node(node * J_NOT_NULL n) {
    J_REQUIRE(nodes.emplace(n->name, n).second, "Duplicate node {}", n->name);
    return n;
  }

  J_A(NODISC,RNN) val_type * module::get_val_type(strings::const_string_view name) {
    auto p = nodes.emplace(name);
    if (p.second) {
      p.first->second = &pool.emplace<val_type>(name);
    } else {
      J_REQUIRE(p.first->second->type == node_value_type, "Expected {} to be a value type.", name);
    }
    return (val_type*)p.first->second;
  }

  void module::define_val_const(s::const_string_view type_name, s::const_string_view name, s::string && value) {
    auto p = val_consts.emplace(name, static_cast<s::string &&>(value), get_val_type(type_name));
    J_REQUIRE(p.second, "Duplicate const {}", name);
  }

  [[nodiscard]] const val_const & module::get_val_const(strings::const_string_view name) const {
    return val_consts.at(name);
  }

  [[nodiscard]] const val_const * module::maybe_get_val_const(strings::const_string_view name) const {
    return val_consts.maybe_at(name);
  }

  J_A(NODISC,RNN) val_type * module::define_val_type(val_region allowed_regions, strings::const_string_view name, doc_comment && comment) {
    val_type* t = get_val_type(name);
    t->allowed_regions = allowed_regions;
    t->comment = static_cast<doc_comment &&>(comment);
    t->index = value_type_ptrs.size();
    value_type_ptrs.push_back(t);
    return t;
  }

  J_A(NODISC,RNN) enum_def * module::define_enum(bool is_flags, strings::const_string_view name, doc_comment && comment) {
    enum_def * e = &pool.emplace<enum_def>(is_flags, name, static_cast<doc_comment &&>(comment));
    J_REQUIRE(nodes.emplace(name, e).second, "Duplicate enum{}", name);
    return e;
  }

  J_A(NODISC,RNN) fn * module::define_fn(strings::const_string_view name, doc_comment && comment) {
    fn * t = &pool.emplace<fn>(name, static_cast<doc_comment &&>(comment));
    J_REQUIRE(nodes.emplace(name, t).second, "Duplicate fn {}", name);
    return t;
  }

  J_A(NODISC,RNN) term * module::define_term(strings::const_string_view name, doc_comment && comment) {
    term * t = &pool.emplace<term>(name, static_cast<doc_comment &&>(comment));
    t->index = term_ptrs.size();
    term_ptrs.push_back(t);
    J_REQUIRE(nodes.emplace(name, t).second, "Duplicate term {}", name);
    return t;
  }

  J_A(NODISC,RNN) nonterminal * module::define_generated_nonterminal(nt_p J_NOT_NULL source_nt) noexcept {
    nonterminal * r = &pool.emplace<nonterminal>(*source_nt);
    r->index = nonterminal_ptrs.size();
    nonterminal_ptrs.push_back(r);
    i32_t num = 1;
    do {
      r->name = s::format("{}{}", source_nt->name, num);
      ++num;
    } while (!nodes.emplace(r->name, r).second);
    return r;
  }

  J_A(NODISC,RNN) nonterminal * module::define_nonterminal(strings::const_string_view name, doc_comment && comment) {
    nonterminal * r = &pool.emplace<nonterminal>(name, static_cast<doc_comment &&>(comment));
    r->index = nonterminal_ptrs.size();
    nonterminal_ptrs.push_back(r);
    J_REQUIRE(nodes.emplace(name, r).second, "Duplicate nonterminal {}", name);
    return r;
  }

  J_A(NODISC,RNN) generated_file_set * module::define_gen_file_set(strings::const_string_view name, doc_comment && comment) {
    generated_file_set * r = &pool.emplace<generated_file_set>(name, static_cast<doc_comment &&>(comment));
    r->index = gen_file_set_ptrs.size();
    gen_file_set_ptrs.push_back(r);
    J_REQUIRE(nodes.emplace(name, r).second, "Duplicate file set {}", name);
    return r;
  }

  J_A(NODISC,RNN) doc_comment * module::define_section(doc_comment && comment) noexcept {
    doc_comment * result = ::new doc_comment(static_cast<doc_comment &&>(comment));
    result->section = nullptr;
    sections.push_back(result);
    return result;
  }

  [[nodiscard]] expr_scope_root module::make_root_scope() noexcept {
    return expr_scope_root(this, root_scope_init);
  }

  J_A(RNN,NODISC) const files::path * module::register_source_file(const files::path & path) noexcept {
    for (const files::path * p : source_names) {
      if (*p == path) {
        return p;
      }
    }
    return source_names.emplace_back(::new files::path(path));
  }

  [[nodiscard]] strings::string module::format_source_location(const source_location & loc) const noexcept {
    s::string result;
    if (loc.cur_file || loc.column >= 0 || loc.line >= 0) {
      result = s::format(" at {}", loc.cur_file ? loc.cur_file->basename() : "Unknown");
      if (loc.line >= 0) {
        result += s::format(":{}:{}", loc.line, loc.column);
      }
    }
    return result;
  }

  [[noreturn]] void module::throw_at(strings::string message, const source_location & loc) const noexcept {
    message += format_source_location(loc);
    J_THROW("{}", message);
  }

  void module::set_global(strings::const_string_view name, attr_value value) {
    root_scope_init.set(name, ::new expr(static_cast<attr_value &&>(value)));
  }
  void module::set_global_bool(strings::const_string_view name, bool value) {
    set_global(name, attr_value(value));
  }
  void module::set_global_int(strings::const_string_view name, i64_t value) {
    set_global(name, attr_value(value));
  }
  void module::set_global_string(strings::const_string_view name, strings::const_string_view value) {
    set_global(name, attr_value(value));
  }
}
