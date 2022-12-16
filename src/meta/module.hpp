#pragma once

#include "meta/rules/term_expr.hpp"
#include "meta/rules/meta_interface.hpp"
#include "meta/rules/nt_data_type.hpp"
#include "meta/value_types.hpp"
#include "streams/string_sink.hpp"
#include "containers/ptr_set.hpp"
#include "files/paths/path.hpp"
#include "mem/bump_pool.hpp"
#include "strings/string_map_fwd.hpp"
#include "meta/expr_scope.hpp"

J_DECLARE_EXTERN_STRING_MAP(j::meta::node*);
J_DECLARE_EXTERN_STRING_MAP(j::meta::te_fn_set);
J_DECLARE_EXTERN_STRING_MAP(j::meta::term_stack_info*);

namespace j::lisp::env {
  struct environment;
}
namespace j::meta::inline rules {
  struct grammar;
  struct test_suite;
  struct test_id_resolver;
  struct static_id_resolver;
}
namespace j::meta {
  struct val_const final {
    J_BOILERPLATE(val_const, CTOR_NE_ND)
    val_const(strings::string && value, val_type * J_NOT_NULL type) noexcept;

    strings::string value{};
    val_type * type = nullptr;
  };
}

J_DECLARE_EXTERN_STRING_MAP(j::meta::val_const);

namespace j::meta {
  struct enum_def;
  struct tpl_expansion_context;
  struct source_location;

  struct module final {
    using nodes_t = ptr_array<node*>;

    J_BOILERPLATE(module, COPY_DEL)
    explicit module(const files::path & base_dir) noexcept;
    ~module();

    strings::string_map<node*> nodes{};
    strings::string_map<val_const> val_consts{};
    strings::string_map<te_fn_set> te_functions{};
    strings::string_map<term_stack_info*> term_stacks{};
    nodes_t term_ptrs{};
    noncopyable_vector<rule *> rules{};
    nodes_t nonterminal_ptrs{};
    nodes_t value_type_ptrs{};
    nodes_t gen_file_set_ptrs{};
    noncopyable_vector<test_suite*> test_suites{};
    noncopyable_vector<doc_comment*> sections{};
    nt_data_types nt_types{};
    grammar * grammar = nullptr;

    attr_context * attr_ctx = nullptr;
    files::path base_dir{};
    expr_scope_init root_scope_init{};

    val_type * trs_bool_type = nullptr;
    val_type * trs_id_type = nullptr;
    val_type * trs_term_type = nullptr;
    val_type * trs_terms_type = nullptr;
    val_type * trs_term_type_type = nullptr;
    term * trs_const_term = nullptr;
    trs_config trs_config{};

    lisp::env::environment * test_env;
    static_id_resolver * normal_resolver;
    test_id_resolver * test_resolver;

    mem::shared_ptr<streams::string_sink> string_sink{};
    strings::formatted_sink formatted_string_sink;
    code_writer string_sink_writer;

    j::mem::bump_pool pool;

    noncopyable_vector<const files::path*> source_names{};
    strings::const_string_view debug_node{};

    [[nodiscard]] span<node* const> nodes_by_type(node_type t) noexcept;

    [[nodiscard]] span<const node* const> nodes_by_type(node_type t) const noexcept;

    [[nodiscard]] span<const generated_file_set* const> gen_file_sets() const noexcept;

    [[nodiscard]] span<term*> terms() noexcept;
    [[nodiscard]] span<const term* const> terms() const noexcept;

    [[nodiscard]] span<nonterminal*> nonterminals() noexcept;
    [[nodiscard]] span<const nonterminal* const> nonterminals() const noexcept;

    [[nodiscard]] span<val_type*> val_types() noexcept;
    [[nodiscard]] span<const val_type* const> val_types() const noexcept;

    J_A(NODISC,RNN) node * node_at(strings::const_string_view name, node_type type = node_any);
    J_A(NODISC,RNN) const node * node_at(strings::const_string_view name, node_type type = node_any) const;
    J_A(NODISC) node * node_maybe_at(strings::const_string_view name, node_type type = node_any) noexcept;
    J_A(NODISC) const node * node_maybe_at(strings::const_string_view name, node_type type = node_any) const noexcept;
    J_A(RNN) node * define_node(node * J_NOT_NULL n);

    J_A(NODISC,RNN) fn * define_fn(strings::const_string_view name, doc_comment && comment);
    J_A(NODISC,RNN) term * define_term(strings::const_string_view name, doc_comment && comment);

    J_A(NODISC,RNN) nonterminal * define_generated_nonterminal(nt_p J_NOT_NULL source_nt) noexcept;
    J_A(NODISC,RNN) nonterminal * define_nonterminal(strings::const_string_view name, doc_comment && comment);
    J_A(NODISC,RNN) generated_file_set * define_gen_file_set(strings::const_string_view name, doc_comment && comment);

    J_A(NODISC,RNN) val_type * get_val_type(strings::const_string_view name);
    J_A(NODISC,RNN) val_type * define_val_type(val_region allowed_regions, strings::const_string_view name, doc_comment && comment);
    void define_val_const(strings::const_string_view type_name, strings::const_string_view name, strings::string && value);
    [[nodiscard]] const val_const & get_val_const(strings::const_string_view name) const;
    [[nodiscard]] const val_const * maybe_get_val_const(strings::const_string_view name) const;

    J_A(NODISC,RNN) enum_def * define_enum(bool is_flags, strings::const_string_view name, doc_comment && comment);

    J_A(NODISC,RNN) doc_comment * define_section(doc_comment && comment) noexcept;

    J_A(NODISC,RNN) const files::path * register_source_file(const files::path & path) noexcept;

    [[nodiscard]] strings::string format_source_location(const source_location & loc) const noexcept;
    [[noreturn]] void throw_at(strings::string message, const source_location & loc) const noexcept;

    strings::string expand(const codegen_template & tpl, expr_scope & scope);

    void set_global(strings::const_string_view name, attr_value value);
    void set_global_bool(strings::const_string_view name, bool value);
    void set_global_int(strings::const_string_view name, i64_t value);
    void set_global_string(strings::const_string_view name, strings::const_string_view value);

    void parse_path(const files::path & path);
    void parse_path(strings::const_string_view path);

    [[nodiscard]] expr_scope_root make_root_scope() noexcept;

    void initialize() noexcept;
  };

  void parse_file(const files::path & path, module * J_NOT_NULL mod);
}
