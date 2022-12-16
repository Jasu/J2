#include "exceptions/assert.hpp"
#include "meta/expr_scope.hpp"
#include "meta/rules/aliases.hpp"
#include "hzd/ctype.hpp"
#include "meta/rules/test_case.hpp"
#include "meta/rules/parse_test_case.hpp"
#include "exceptions/assert.hpp"
#include "meta/rules/rule.hpp"
#include "meta/rules/nonterminal.hpp"
#include "containers/hash_map.hpp"
#include "meta/fn.hpp"
#include "meta/term.hpp"
#include "meta/gen_file.hpp"
#include "meta/attr_value_interfaces.hpp"
#include "meta/expr.hpp"
#include "files/fds/fd.hpp"
#include "meta/attr_context.hpp"
#include "files/fds/io.hpp"
#include "files/fds/open.hpp"
#include "files/memory_mapping.hpp"
#include "files/paths/path.hpp"
#include "meta/comments.hpp"
#include "meta/cpp_codegen.hpp"
#include "meta/module.hpp"
#include "meta/parsing.hpp"
#include "strings/format.hpp"
#include "strings/parsing/parse_int.hpp"
#include "lisp/env/context.hpp"

/*!max:re2c*/
#ifndef YYMAXFILL
#define YYMAXFILL 5
#endif

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused"

namespace s = j::strings;

namespace j::meta {
  base_parser_state::base_parser_state(const char * J_NOT_NULL at, const char * J_NOT_NULL end,
                                       const files::path * J_NOT_NULL path, module * J_NOT_NULL mod,
                                       s::const_string_view default_package, alias_table * aliases) noexcept
    : cursor(at), limit(end), line_begin(at), aliases(alias_table::create(aliases)), mod(mod), cur_path(path), default_package(default_package)
  { }

  base_parser_state::~base_parser_state() {
    alias_table::destroy(aliases);
  }


  J_A(RNN,NODISC) term * base_parser_state::term_at(strings::const_string_view name) const {
    alias * a = aliases->maybe_at(name);
    if (a && a->type == alias_type::term) {
      return a->alias_term;
    }
    node * n = mod->node_maybe_at(name, node_term);
    check(n, "Unknown term", name);
    return (term*)n;
  }

  J_A(RNN,NODISC) nt_p base_parser_state::nt_at(strings::const_string_view name) const {
    node * n = mod->node_maybe_at(name, node_nonterminal);
    check(n, "Unknown nonterminal", name);
    return (nt_p)n;
  }

  J_A(NODISC) term_expr * base_parser_state::term_expr_macro_maybe_at(strings::const_string_view name) const {
    alias * a = aliases->maybe_at(name);
    return a && a->type == alias_type::macro ? a->get_term_expr() : nullptr;
  }
  J_A(NODISC) matcher_base * base_parser_state::matcher_macro_maybe_at(strings::const_string_view name) const {
    alias * a = aliases->maybe_at(name);
    return a && a->type == alias_type::macro ? a->get_matcher() : nullptr;
  }

  [[noreturn]] J_A(NI,COLD) void base_parser_state::fail(s::const_string_view msg, s::const_string_view ctx) const {
    J_THROW("{} in {}:{}:{}{}{} ({u}·{u})", msg, cur_path->basename(), line_number, (i32_t)(cursor - line_begin + 1), ctx ? " " : "", ctx,
            s::const_string_view(j::max(line_begin, cursor - 5), cursor),
            s::const_string_view(cursor, cursor + 5));
  }

  J_A(NI) void base_parser_state::log_ctx() const noexcept {
    J_DEBUG(" in {}:{}:{}", cur_path->basename(), line_number, (i32_t)(cursor - line_begin + 1));
  }

  namespace {

#define WITH_PARSER_STATES(M)        \
    M(root)M(term)                   \
    M(before_body)M(body) \
    M(op_list)M(op_before)M(op_after)

    enum parser_status : u8_t {
      st_none,
#define STATE_ENUM(N) st_##N,
      WITH_PARSER_STATES(STATE_ENUM)
#undef STATE_ENUM
      num_states,
    };

    J_A(ND) inline constexpr const char * state_names[num_states] = {
      [st_none] = "None",
#define STATE_NAME(N) [st_##N] = #N,
      WITH_PARSER_STATES(STATE_NAME)
#undef STATE_NAME
    };

    enum keyword_subtype : u16_t {
      sub_none,

      sub_term = node_term,
      sub_op = node_term_member,
      sub_any = node_any,

      sub_in = region_input,
      sub_const = region_data_or_const,

      sub_flags = 1U,

      sub_term_type = sub_op,
      sub_term_stack_alloc = sub_op + 1,
      sub_terms = sub_any,
      sub_bool = sub_any + 1,
      sub_primary_term_type = sub_bool + 1,
      sub_primary_terms = sub_bool + 2,
      sub_id = sub_bool + 3,

      sub_macro = sub_op,
    };

    struct J_TYPE_HIDDEN state;

    struct J_TYPE_HIDDEN keyword {
      s::const_string_view name;
      u32_t allowed_states;
      keyword_subtype subtype = sub_none;
      void (state::*callback)(keyword_subtype);
    };


    constexpr inline u32_t term_ctor_op_states_v J_A(ND) = (1 << st_op_before) | (1 << st_op_list);
    constexpr inline u32_t prop_states_v J_A(ND) = (1 << st_before_body) | (1 << st_body) | (1 << st_op_after);

    struct J_TYPE_HIDDEN state final : base_parser_state {
      parser_status stack[8] {st_root};
      u8_t depth = 1;

      test_suite * cur_test_suite = nullptr;
      attr_value value_stack[4]{};
      u8_t value_depth = 1U;

      expr_scope_root root;
      hash_map<node*, expr*> deferred_exprs;
      s::const_string_view log_node{};

      J_A(AI,NODISC) inline term & cur_term() const {
        return value_stack[value_depth - 1].as_term();
      }

      J_A(AI,NODISC) inline generated_file_set & cur_gen_file_set() const {
        return value_stack[value_depth - 1].as_gen_file_set();
      }

      void exit_body(char c) {
        check(stack[depth] != st_body || c == '}', "Unexpected ;");
        auto cur_state = stack[--depth];
        check(depth > 0, "Exited too many times.");
        if (cur_test_suite) {
          aliases = alias_table::destroy(aliases);
          J_ASSERT_NOT_NULL(aliases);
          cur_test_suite = nullptr;
          return;
        }
        --value_depth;
        if (log_node && value_stack[value_depth].is_node()
            && value_stack[value_depth].as_node().name == log_node) {
          log("Stopping log for {}", log_node);
          disable_log();
        }
      }

      J_A(ND,NI) void enter_node_body(node * n) noexcept {
        check(!cur_test_suite, "Nested in suite");
        if (n && log_node && n->name == log_node) {
          enable_log();
          log("Starting log for {}", n->name);
        }
        value_stack[value_depth++] = n ? attr_value(n) : attr_value();
        stack[depth++] = st_before_body;
      }

      void on_keyword(const char * J_NOT_NULL begin) {
        const s::const_string_view kwd{begin, cursor};
        const u32_t mask = 1U << stack[depth - 1U];
        for (auto & k : kwd_map) {
          if (k.name == kwd && k.allowed_states & mask) {
            (this->*k.callback)(k.subtype);
            return;
          }
        }
        if (mask & term_ctor_op_states_v) {
          on_term_ctor_op(kwd);
        } else if (mask & prop_states_v) {
          on_prop(kwd);
        } else {
          [[unlikely]];
          fail("Unexpected {}.", kwd);
        }
      }

      state(const char * J_NOT_NULL at, const char * J_NOT_NULL end,
            const files::path & path, module * J_NOT_NULL mod,
            state * parent, alias_table * aliases) noexcept
        : base_parser_state(at, end, mod->register_source_file(path), mod, "test", aliases),
          root(mod, {}),
          log_node(mod->debug_node),
          parent(parent)
      {
      }

      doc_comment next_comment;
      doc_comment * section = nullptr;
      state * parent = nullptr;

      J_A(NI,COLD,NORET) void fail(strings::const_string_view str) const {
        base_parser_state::fail(str);
      }

      J_A(NI,COLD,NORET) void fail(const char * J_NOT_NULL str) const {
        base_parser_state::fail(str);
      }

      J_A(NI,COLD,NORET) void fail(const char * J_NOT_NULL msg, s::const_string_view n) const {
        fail(s::format(msg, n));
      }

      J_A(AI,ND) inline void check(bool cond, const char * J_NOT_NULL msg, s::const_string_view n) const {
        if (!cond) {
          fail(msg, n);
        }
      }

      J_A(AI,ND) inline void check(bool cond, const char * J_NOT_NULL msg) const {
        if (!cond) {
          fail(msg);
        }
      }

      void parse_root() {
        const char *begin;
        loop:
        begin = cursor;
        /*!re2c:main
          re2c:api:style             = free-form;
          re2c:define:YYCTYPE        = "char";
          re2c:define:YYCURSOR       = "cursor";
          re2c:define:YYLIMIT        = "limit";
          re2c:define:YYFILL         = "J_UNREACHABLE();";

          // The buffer to be parsed is the whole input + 0-terminator.
          // (0-terminator is implied by the lack of re2c:eof)
          // re2c:yyfill:enable = 0;

          [\x00] {
            goto out;
          }

          [ ]+ { goto loop; }
          "\n" { newline(); goto loop; }

          [;}] {
            exit_body(cursor[-1]);
            goto loop;
          }
          "," {
            set_state(st_op_before, st_op_after);
            --value_depth;
            goto loop;
          }
          ")" {
            if (stack[depth - 1] == st_op_after) {
              --value_depth;
            } else {
              check(stack[depth - 1] == st_op_list, "Unexpected close paren");
            }
            stack[depth - 1] = st_before_body;
            goto loop;
          }

          "{" {
            set_state(st_body, st_before_body);
            goto loop;
          }

          "#" [#]{0,2} {
            on_comment(cursor - begin == 3U);
            goto loop;
          }

          "@"? [A-Z][A-Za-z0-9]* {
            on_keyword(begin);
            goto loop;
          }

          * { fail("Unexpected token", strings::const_string_view(cursor - 1, 1)); }
        */
      out:
        if (depth != 1) {
          J_THROW("Unexpected EOF, level {} at {}", state_names[(i32_t)stack[depth]], depth);
        }

        {
          auto scope = mod->make_root_scope();
          for (auto & c : deferred_exprs) {
            auto res = eval_expr(scope, c.second);
            if (c.first->type == node_term_member) {
              switch (res.type) {
              case attr_term_set:
                static_cast<term_member*>(c.first)->allowed_children = res.node_set;
                break;
              case attr_term:
                static_cast<term_member*>(c.first)->allowed_children.add(*res.node);
                break;
              default:
                fail("Term children evaluated to an invalid value.");
              }
            } else {
              J_TODO();
            }
          }
        }
      }

      void on_comment(bool is_divider) {
        auto comment_line = parse_to_eol(*this);
        if (is_divider) {
          if (next_comment) {
            section = mod->define_section(static_cast<doc_comment &&>(next_comment));
            next_comment.section = section;
          }
          next_comment.clear();
          return;
        }
        const char * start = comment_line.begin();
        const char * end = comment_line.end();
        if (start != end && *start == ' ') {
          ++start;
        }
        while (start != end && end[-1] == ' ') {
          --end;
        }
        next_comment.on_comment_line({start, end});
      }

      inline void set_state(parser_status to, parser_status from) {
        check(depth > 1 && from == stack[depth - 1], "Invalid transition.");
        stack[depth - 1] = to;
      }

      void on_term_ctor_op(s::const_string_view kwd) {
        struct val_type * type = (val_type*)mod->node_at(kwd, node_value_type);
        push_term_op((val_region)(type->allowed_regions & region_ctor), type);
        stack[depth - 1U] = st_op_after;
      }

      void on_prop(s::const_string_view name) {
        auto & n = value_stack[value_depth - 1];
        if (n.is_attr_def() && name == "Type") {
          n.as_attr_def().type = parse_attr_type(*this);
        } else {
          attr_type_def * type = nullptr;
          if (name[0] == '@') {
            type = &mod->attr_ctx->defs[mod->attr_ctx->at(name.without_prefix(1))].type;
          }
          interface_of(n).set_prop(root, n, name, maybe_parse_attr_value(*this, type));
        }
      }

      void on_include(keyword_subtype) {
        files::path p{parse_string_out(*this)};
        if (p.is_relative()) {
          p = cur_path->parent() / p;
        }
        const auto buf = files::read_to_end(files::open(p, files::open_flags::read), YYMAXFILL);
        state{buf.begin(), buf.end(), p, mod, this, aliases}.parse_root();
      }

      void on_term(keyword_subtype) {
        check(!cur_test_suite, "Nested in suite");
        auto term = mod->define_term(parse_id(*this), static_cast<doc_comment &&>(next_comment));
        value_stack[value_depth++] = attr_value((node*)term);
        const char ch = skip_ws_past(*this, "=(");
        stack[depth++] = ch == '(' ? st_op_list : st_before_body;
        if (ch == '=') {
          term->apply_defaults(*static_cast<struct term*>(term_at(parse_id(*this))));
        }
      }

    void on_term_extend(keyword_subtype) {
      enter_node_body(term_at(parse_id(*this)));
    }

    void on_test_suite(keyword_subtype) {
      aliases = alias_table::create(aliases);
      check(value_depth == 1 && !cur_test_suite, "Test suite must be at root");
      cur_test_suite = ::new test_suite(parse_string_out(*this));
      mod->test_suites.push_back(cur_test_suite);
      stack[depth++] = st_before_body;
    }

    void on_test_case(keyword_subtype) {
      check(cur_test_suite, "Test must be inside suite");
      cur_test_suite->add_test(parse_test_case(*this));
    }

    void on_attr_def(keyword_subtype sub) {
      enter_node_body(&mod->attr_ctx->defs[mod->attr_ctx->define_attr(parse_name(*this), (node_type)sub, static_cast<doc_comment &&>(next_comment))]);
    }

    void on_enum_def(keyword_subtype sub) {
      enter_node_body(mod->define_enum(sub, parse_name(*this), static_cast<doc_comment &&>(next_comment)));
    }

    void on_rule_def(keyword_subtype) {
      check(!cur_test_suite, "Nested in suite");
      rule * r = parse_rule(*this, mod->rules.size());
      if (!r) {
        skip_ws_past(*this, ";");
        return;
      }
      value_stack[value_depth++] = attr_value(r);
      stack[depth++] = st_before_body;
      mod->rules.push_back(r);
    }

    void parse_nonterminal_ctor(nonterminal * J_NOT_NULL nt) {
      check(mod->trs_term_type, "No type defined a Term.");
      skip_ws_past(*this, "(");
      nt_data_type type(nt->name);
      while (true) {
        bool is_term_stack = false;
        char ch = skip_ws(*this);
        switch (ch) {
        case ')':
          mod->nt_types.set_data_type(nt, type);
          return;
        case '~':
          is_term_stack = true;
          ++cursor;
          [[fallthrough]];
        default: {
          --cursor;
          s::const_string_view name = parse_name(*this, "in nt member name");
          val_type * cur_type;
          term_expr * initializer = nullptr;
          term_expr * extra_arg = nullptr;
          i32_t tracking_index = -1;
          if (is_term_stack) {
            const term_stack_info * ts = mod->term_stacks.at(name);
            cur_type = ts->allocation_type;
            J_ASSERT_NOT_NULL(cur_type, cur_type->release, cur_type->allocate);
            term_exprs args;
            if (ts->extra_arg) {
              args.emplace_back(copy(ts->extra_arg));
              extra_arg = ts->extra_arg;
            }
            tracking_index = ts->tracking_index;
            initializer = ::new term_expr(te_global_fn_call{cur_type->allocate, static_cast<term_exprs &&>(args)});
          } else {
            if (*cursor == ':') {
              ++cursor;
              cur_type = (val_type*)mod->node_at(parse_name(*this, "parsing nonterminal member type name"), node_value_type);
            } else {
              cur_type = mod->trs_term_type;
            }
            if (skip_ws(*this) == '=') {
              te_type t(cur_type);
              initializer = parse_term_expr(*this, nullptr, &t);
            } else {
              check(!is_term_stack, "Initializer is required for term stack fields");
              --cursor;
            }
          }

          type.add_field(cur_type, name, initializer, extra_arg, tracking_index);
          break;
        }
        }
      }
    }

    void on_nonterminal_def(keyword_subtype) {
      nonterminal * nonterminal = mod->define_nonterminal(parse_name(*this), static_cast<doc_comment &&>(next_comment)); parse_nonterminal_ctor(nonterminal);
      enter_node_body(nonterminal);
    }

    void on_file_def(keyword_subtype) {
      enter_node_body(mod->define_gen_file_set(parse_id(*this), static_cast<doc_comment &&>(next_comment)));
    }

    void on_value_type(keyword_subtype sub) {
      enter_node_body(mod->define_val_type((val_region)sub, parse_name(*this), static_cast<doc_comment &&>(next_comment)));
    }

    J_A(AI,ND,NODISC,HIDDEN,RNN) inline val_type * cur_val_type() {
      return &value_stack[value_depth - 1].as_val_type();
    }

    J_A(AI,ND,NODISC,HIDDEN,RNN) inline nt_p cur_nt() {
      return &value_stack[value_depth - 1].as_nonterminal();
    }

    void on_is_trs_type(keyword_subtype st) {
      val_type * type = cur_val_type();
      switch (st) {
      case sub_primary_term_type:
        mod->trs_term_type = type;
        [[fallthrough]];
      case sub_term:
        type->is_trs_term_type = true;
        break;
      case sub_primary_terms:            mod->trs_terms_type = type; [[fallthrough]];
      case sub_terms:            type->is_trs_terms_type = true; break;
      case sub_term_type: mod->trs_term_type_type = type; break;
      case sub_bool:             mod->trs_bool_type = type; break;
      case sub_id:             mod->trs_id_type = type; break;
      default: fail("Unknown st");
      }
    }

    void on_term_stack(keyword_subtype) {
      val_type * type = cur_val_type();
      auto name = parse_name(*this);
      skip_ws_past(*this, "{");
      term_stack_info * info = ::new term_stack_info{.name = name, .allocation_type = type, .tracking_index = (i32_t)mod->term_stacks.size()};
      while (skip_ws(*this) != '}') {
        --cursor;
        auto n = parse_name(*this);
        if (n == "ExtraArg") {
          info->extra_arg = parse_term_expr(*this);
        } else if (n == "DebugName") {
          info->debug_name = parse_string_out(*this);
        } else {
          fail("Unknown property", n);
        }
        skip_ws_past(*this, ";");
      }
      mod->term_stacks.emplace(name, info);
      type->term_stacks.push_back(info);
    }

    void on_lifecycle(keyword_subtype) {
      val_type * type = cur_val_type();
      skip_ws_past(*this, "{");
      while (skip_ws(*this) != '}') {
        --cursor;
        auto n = parse_name(*this);
        auto v = parse_name(*this);
        if (n == "Release") {
          type->release = v;
        } else if (n == "Allocate") {
          type->allocate = v;
        } else {
          fail("Unsupported life cycle property \"{}\".", n);
        }
        skip_ws_past(*this, ";");
      }
    }

    void on_from(keyword_subtype) {
      val_type * vt = mod->get_val_type(parse_name(*this));
      cur_val_type()->conversions.push_back(pair<val_type*, codegen_template>{vt, parse_template_out(*this)});
    }

    void on_from_attr(keyword_subtype) {
      attr_type_def at = parse_attr_type(*this);
      cur_val_type()->attr_conversions.push_back(pair<attr_type_def, codegen_template>{at, parse_template_out(*this)});
    }

    void on_parse_as(keyword_subtype) {
      val_type * cur = cur_val_type();
      check(!cur->parse_as, "ParseAs specified twice.");
      cur->parse_as = parse_attr_type(*this);
    }

    void on_term_op(keyword_subtype) {
      enter_node_body(cur_term().get_member(parse_id(*this)));
    }

    void on_term_op_children(keyword_subtype) {
      auto & m = value_stack[value_depth - 1].as_term_member();
      check(m.region == region_input, "Tried to set children for a non-input");
      check(deferred_exprs.emplace(&m, parse_expr(*this)).second, "Term member already has children");
      skip_ws_past(*this, ";");
    }

    void on_var_def(keyword_subtype) {
      auto name = parse_name(*this, "parsing Def name");
      skip_ws_past(*this, "=");
      cur_scope_init().set(name, parse_expr(*this));
      skip_ws_past(*this, ";");
    }

    void on_value_def(keyword_subtype) {
      auto name = parse_name(*this, "parsing Value name");
      skip_ws_past(*this, ":");
      auto type_name = parse_name(*this, "parsing Value type name");
      skip_ws_past(*this, "=");
      mod->define_val_const(type_name, name, parse_string_out(*this));
      skip_ws_past(*this, ";");
    }

    expr_scope_init & cur_scope_init() {
      if (value_depth == 1U) {
        return mod->root_scope_init;
      }
      return cur_gen_file_set().scope_init;
    }

    void on_alias_def(keyword_subtype st) {
      s::const_string_view name = parse_id(*this, "parsing alias name");
      if (st == sub_term) {
        aliases->add_alias(name, term_at(parse_id(*this, "parsing alias term name")));
      } else {
        J_ASSERT(st == sub_macro);
        auto snap = snapshot();
        term_expr * te = parse_term_expr(*this);
        restore(snap);
        matcher_base * matcher = parse_matcher(*this);
        aliases->add_alias(name, te, matcher);
      }
      skip_ws_past(*this, ";");
    }

    void on_fn_def(keyword_subtype) {
      if (value_depth && value_stack[value_depth - 1].is_val_type()) {
        on_type_fn_def();
        return;
      }
      fn * f = mod->define_fn(parse_name(*this), static_cast<doc_comment &&>(next_comment));
      skip_ws_past(*this, "(");
      bool is_rest = false;
      for (;;) {
        switch (skip_ws(*this)) {
        case ',': continue;
        case '.':
          check(cursor[0] == '.' && cursor[1] == '.', "Expected full '...' in Fn {} (", f->name);
          cursor += 2U;
          is_rest = true;
          continue;
        case ')': goto out;
        default:
          --cursor;
          f->push_arg(parse_name(*this), is_rest);
          is_rest = false;
        }
      }
    out:
      check(!is_rest, "Expected arg name after ... in {}", f->name);
      skip_ws_past(*this, "=");
      f->e = parse_expr(*this);
      skip_ws_past(*this, ";");
      cur_scope_init().define_fn(f);
    }

    [[nodiscard]] te_type parse_te_type(val_type * default_type) {
      if (*cursor != ':') {
        return default_type ? default_type : te_type::any;
      }
      ++cursor;
      auto name = parse_name(*this, "When parsing arg type");
      if (attr_type_def at = maybe_parse_attr_simple_type(*this, name)) {
        return te_type(static_cast<attr_type_def &&>(at));
      } else if (val_type * vt = (val_type*)mod->node_maybe_at(name, node_value_type)) {
        return te_type(vt);
      } else {
        fail("Type not found by {}", name);
      }
    }

    [[nodiscard]] te_fn_args parse_te_args(val_type * this_type, val_type * default_arg_type) {
      bool had_variadic = false;
      skip_ws_past(*this, "(");
      te_fn_args args;
      for (;;) {
        switch (skip_ws(*this)) {
        case ')': return args;
        default: {
          --cursor;
          check(!had_variadic, "Variadic argument must be last");
          auto name = parse_name(*this, "When parsing arg name");
          te_type type = this_type && name == "This" ? this_type : parse_te_type(default_arg_type);
          if (this_type && !args && name != "This") {
            args.emplace_back(this_type, "This");
          }
          te_function_arg & arg = args.emplace_back(static_cast<te_type &&>(type), name);
          arg.flags = maybe_parse_te_fn_arg_options();
          if (*cursor == '.') {
            check(cursor[1] == '.' && cursor[2] == '.', "Expected full ... after var name");
            cursor += 3;
            arg.flags = (te_fn_arg_flags)(arg.flags | arg_variadic);
            had_variadic = true;
          }
        }
        }
      }
    }

    [[nodiscard]] te_fn_arg_flags maybe_parse_te_fn_arg_options() {
      i32_t result = no_fn_arg_flags;
      if (skip_ws(*this) == '[') {
        while (skip_ws(*this) != ']') {
          --cursor;
          auto n = parse_name(*this);
          if (n == "MoveSrc") {
            result |= lifecycle_move_src;
          } else if (n == "MoveDst") {
            result |= arg_written;
            result |= lifecycle_move_dst;
          } else if (n == "Consume") {
            result |= arg_written;
            result |= lifecycle_consume;
          } else if (n == "Return") {
            result |= arg_written;
            result |= lifecycle_returned;
          } else if (n == "Use") {
            result |= lifecycle_use;
          } else if (n == "Write") {
            result |= arg_written;
          } else {
            fail("Unknown attribute {}", n);
          }
        }

      } else {
        --cursor;
      }
      return (te_fn_arg_flags)result;
    }

    [[nodiscard]] pair<i32_t, u8_t> maybe_parse_te_fn_options() {
      pair<i32_t, u8_t> result{100, no_fn_flags};
      if (skip_ws_past(*this, "[=") == '[') {
        while (skip_ws(*this) != ']') {
          --cursor;
          auto n = parse_name(*this, "parsing reduction attribute");
          if (n == "Precedence") {
            result.first = parse_attr_value(*this).as_int();
          } else if (n == "Acquire") {
            result.second |= lifecycle_acquire;
          } else {
            fail("Unknown attribute {}", n);
          }
        }
        skip_ws_past(*this, "=");
      }
      return result;
    }

    [[nodiscard]] te_function parse_te_fn(val_type * default_result_type, val_type * this_type, val_type * default_arg_type) {
      te_fn_args args = parse_te_args(this_type, default_arg_type);
      te_type result_type = parse_te_type(default_result_type);
      auto opts = maybe_parse_te_fn_options();
      codegen_template tpl = parse_template_out(*this);
      skip_ws_past(*this, ";");
      bool has_move_src = false, has_move_dst = false;
      for (auto & arg : args) {
        if (arg.is_variadic()) {
          opts.second |= fn_variadic;
        }
        if (arg.is_move_src()) {
          check(!has_move_src, "Multiple move sources");
          has_move_src = true;
        }
        if (arg.is_move_dst()) {
          check(!has_move_dst, "Multiple move destinations");
          has_move_dst = true;
        }
      }
      check(has_move_dst == has_move_src, "Must have both move src and dst");
      return te_function(static_cast<te_type &&>(result_type),
                         static_cast<codegen_template &&>(tpl),
                         static_cast<te_fn_args &&>(args),
                         (te_fn_flags)opts.second,
                         opts.first);
    }

    void on_type_fn_def() {
      auto name = parse_name(*this, "When parsing type member fn");
      val_type * vt = cur_val_type();
      vt->define_fn(name, parse_te_fn(nullptr, vt, nullptr));
    }

    void on_type_prop_def(keyword_subtype) {
      auto name = parse_name(*this, "When parsing type member fn");
      skip_ws_past(*this, ":");
      val_type * type = mod->get_val_type(parse_name(*this, "When parsing result type"));
      i32_t precedence = 100;
      if (skip_ws_past(*this, "[=") == '[') {
        while (skip_ws(*this) != ']') {
          --cursor;
          auto n = parse_name(*this);
          if (n == "Precedence") {
            precedence = parse_attr_value(*this).as_int();
          } else {
            fail("Unknown attribute {}", n);
          }
        }
        skip_ws_past(*this, "=");
      }
      cur_val_type()->define_prop(name, parse_template_out(*this), type, precedence);
      skip_ws_past(*this, ";");
    }

    J_A(ND) void push_term_op(val_region region, node * J_NOT_NULL val_type) {
      check(!cur_test_suite, "Nested in suite");
      auto val_type_ = (struct val_type*)val_type;
      value_stack[value_depth++] = attr_value(cur_term().add_member(parse_id(*this, "Parsing op name"), static_cast<doc_comment &&>(next_comment),
                                                                    region, val_type_));
    }

    void on_term_data(keyword_subtype) {
      auto n = parse_name(*this);
      push_term_op(region_data, mod->node_at(n, node_value_type));
      stack[depth++] = st_before_body;
    }

    void on_is_const(keyword_subtype) {
      mod->trs_const_term = &cur_term();
      skip_ws_past(*this, ";");
    }

    void on_operator_def(keyword_subtype) {
      operator_type op = parse_te_operator(*this);
      val_type * cur_type = cur_val_type();
      val_type * default_type = cur_type;
      switch (op) {
      case operator_type::le: case operator_type::lt:
      case operator_type::ge: case operator_type::gt:
      case operator_type::eq: case operator_type::neq:
        default_type = mod->trs_bool_type;
        break;
      default: break;
      }

      te_function & fn = cur_type->define_operator(op, parse_te_fn(default_type, cur_type, cur_type));
      check(fn.args.size() == 2, "Expected a single non-`this` argument for operator.");
    }

    void on_trs(keyword_subtype) {
      skip_ws_past(*this, "{");
      while (skip_ws(*this) != '}') {
        --cursor;
        auto n = parse_name(*this);
        if (n == "Fn") {
          mod->te_functions[parse_name(*this, "When parsing TRS fn")].add(parse_te_fn(mod->trs_term_type, nullptr, nullptr));
        } else if (n == "GetStackTerm") {
          mod->trs_config.get_stack_term = parse_template_out(*this);
        } else if (n == "GetInputTerm") {
          mod->trs_config.get_input_term = parse_template_out(*this);
        } else if (n == "GetStackNonTerminal") {
          mod->trs_config.get_stack_nt = parse_template_out(*this);
        } else if (n == "GetStackNonTerminalField") {
          mod->trs_config.get_stack_nt_field = parse_template_out(*this);
        } else if (n == "ConstructTerm") {
          mod->trs_config.construct_term = parse_template_out(*this);
        } else {
          fail("Unexpected {} in TRS.", n);
        }
      }
    }

    void on_path(keyword_subtype) {
      auto & fs = cur_gen_file_set();
      auto & f = fs.files[fs.num_files++];
      f.path = parse_string_out(*this);
      auto suffix = strrchr(f.path.data(), '.');
      check(suffix, "Path must have an extension.");
      f.suffix = suffix + 1;
      f.tpl = parse_template_out(*this);
    }

#define STM(N) | (1 << st_##N)
#define KWD(N,CB,...) {N, 0 J_FE(STM, __VA_ARGS__), sub_none, &state::on_##CB}
#define KWDS(N,CB,S,...) {N, 0 J_FE(STM, __VA_ARGS__), sub_##S, &state::on_##CB}
      J_A(ND) inline static constexpr const keyword kwd_map[]{
        KWD("Term",       term,        root),
        KWD("Extend",     term_extend, root),
        KWD("TestSuite",  test_suite, root),
        KWD("Test",       test_case, body),

        KWD("IsConst",   is_const, body),

        KWDS("Attr",      attr_def,    any,   root),
        KWDS("TermAttr",  attr_def,    term,  root),
        KWDS("OpAttr",    attr_def,    op,    root),

        KWDS("InType",    value_type,  in,    root),
        KWDS("ConstType", value_type,  const, root),
        KWDS("Type",      value_type,  none, root),

        KWD("Include",    include,     root),

        KWD("NonTerminal", nonterminal_def,    root),
        KWD("Rule",       rule_def,    root),

        KWD("File",       file_def,    root),
        KWD("Path",       path,        body),

        KWDS("Enum",      enum_def,    none,  root),
        KWDS("Flags",     enum_def,    flags, root),

        KWD("TermStack", term_stack, body),
        KWD("LifeCycleTracking", lifecycle, body),

        KWDS("IsBoolType", is_trs_type, bool, body),
        KWDS("IsTermType", is_trs_type, term, body),
        KWDS("IsPrimaryTermType", is_trs_type, primary_term_type, body),
        KWDS("IsIdType", is_trs_type, id, body),
        KWDS("IsTermsType", is_trs_type, terms, body),
        KWDS("IsPrimaryTermsType", is_trs_type, primary_terms, body),
        KWDS("IsTermTypeType", is_trs_type, term_type, body),
        KWDS("IsTermStackAllocationType", is_trs_type, term_stack_alloc, body),

        KWDS("Alias",     alias_def, term,     root, body),
        KWDS("Macro",     alias_def, macro,     root, body),
        KWD("Fn",         fn_def,      root, body),
        KWD("Property",   type_prop_def, body),
        KWD("Operator",   operator_def, body),
        KWD("Def",        var_def,     root, body),
        KWD("Value",      value_def,   root),
        KWD("Op",         term_op,     body),
        KWD("Children",   term_op_children,  body),
        KWD("Data",       term_data,   body),
        KWD("FromAttr",   from_attr,   body),
        KWD("From",       from,   body),
        KWD("ParseAs",    parse_as,   body),
        KWD("TRS",        trs,   root),
      };
    };

  }

  void parse_file(const files::path & path, module * J_NOT_NULL mod) {
    lisp::env::env_ctx_guard g(lisp::env::env_context.enter(mod->test_env));
    const auto buf = files::read_to_end(files::open(path, files::open_flags::read), YYMAXFILL);
    state{buf.begin(), buf.end(), path, mod, nullptr, nullptr}.parse_root();
    mod->initialize();
  }
#pragma clang diagnostic pop
}
