#include "meta/dump.hpp"
#include "meta/expr.hpp"
#include "meta/term.hpp"
#include "meta/basic_node_set.hpp"
#include "meta/comments.hpp"
#include "meta/expr_scope.hpp"
#include "meta/module.hpp"
#include "meta/rules/nonterminal.hpp"
#include "meta/attr_context.hpp"
#include "strings/styling/default_styles.hpp"
#include "meta/node.hpp"
#include "meta/cpp_codegen.hpp"
#include "strings/format.hpp"
#include "streams/string_sink.hpp"

namespace j::meta {
  namespace s = strings;
  namespace st = strings::styles;

  namespace {
    [[nodiscard]] inline s::style node_style(const attr_value & n) noexcept {
      switch (n.as_node().type) {
      case node_term:
        return st::bright_cyan.with_bold();
      case node_fn:
        return st::bright_green.with_bold();
      case node_term_member:
        return st::bright_magenta.with_bold();
      case node_value_type:
        return st::bright_yellow.with_bold();
      case node_attr_def:
        return st::bright_blue.with_bold();
      case node_enum:
        return st::bright_blue.with_bold();
      default:
        return st::error;
      }
    }

    const char * const stmt_begins[num_stmt_types_v]{
      [stmt_foreach]       = "{#bright_green,bold}\\{ ForEach{/} [",
      [stmt_if]            = "{#bright_yellow,bold}\\{ If{/} [",
      [stmt_let]           = "{#bright_magenta,bold}\\{ Let{/} [",
      [stmt_set]           = "{#bright_red,bold}\\{ Set{/} [",
      [stmt_with_writer_settings] = "{#light_gray,bold}\\{ {/}",
      [stmt_indent] = "{#light_gray,bold}\\{ Indent{/}",
      [stmt_indent_inner] = "{#light_gray,bold}\\{ IndentInner{/}",
    };

    const char * const stmt_ends[num_stmt_types_v]{
      [stmt_foreach]       = "{#bright_green,bold}} {#green}{#bold}#{/} End ForEach{/}",
      [stmt_if]            = "{#bright_yellow,bold}} {#yellow}{#bold}#{/} End If{/}",
      [stmt_let]           = "{#bright_magenta,bold}} {#magenta}{#bold}#{/} End Let{/}",
      [stmt_set]           = "{#bright_red,bold}} {#red}{#bold}#{/} End Set{/}",
      [stmt_with_writer_settings] = "{#light_gray,bold}\\}{/}",
      [stmt_indent] = "{#light_gray,bold}\\}{/} {#gray}{#bold}#{/} End Indent{/}",
      [stmt_indent_inner] = "{#light_gray,bold}\\}{/} {#gray}{#bold}#{/} End IndentInner{/}",
    };

    void dump_stmt(dump_context & ctx, const tpl_part & tpl) {
      tpl_stmt_type t = stmt_type(tpl.type);
      ctx.to.begin_line_no_token();
      ctx.to.write_formatted(stmt_begins[t]);

      switch (t) {
      case stmt_if:
        dump(ctx, *tpl.sub.ifelse.e);
        ctx.to.write("]", st::bright_yellow.with_bold());
        break;
      case stmt_set:
      case stmt_let:
      case stmt_foreach:
        if (tpl.sub.let.name) {
          ctx.to.write(tpl.sub.let.name, st::bright_cyan.with_bold());
          ctx.to.write(" = ");
        }
        dump(ctx, *tpl.sub.let.e);
        ctx.to.write("]");
        break;
      case stmt_with_writer_settings:
        if (tpl.sub.writer_settings == cpp_comment) {
          ctx.to.write("Comment", st::light_gray.with_bold());
        } else if (tpl.sub.writer_settings == cpp_doc_comment) {
          ctx.to.write("DocComment", st::light_gray.with_bold());
        } else if (tpl.sub.writer_settings == hash_comment) {
          ctx.to.write("HashComment", st::light_gray.with_bold());
        } else {
          ctx.to.write("UnknownStyle", st::error);
        }
        break;
      case stmt_indent:
      case stmt_indent_inner:
        if (tpl.sub.indent >= 0) {
          ctx.to.write_formatted(" {#bold}{}{/}", tpl.sub.indent);
        }
        break;
      }

      J_ASSERT(tpl.type >= tpl_min_stmt);
      ctx.to.begin_line();
      if (tpl.sub.body) {
        dump(ctx, *tpl.sub.body);
      }
      ctx.to.clear_empty_line();
      if (t == stmt_if && tpl.sub.ifelse.else_body) {
        ctx.to.write("} Else {", st::bright_yellow.with_bold());
        ctx.to.begin_line();
        dump(ctx, *tpl.sub.ifelse.else_body);
      }
      ctx.to.write_formatted(stmt_ends[t]);
      ctx.to.queue_break();
    }

    [[nodiscard]] inline bool is_short_tuple(span<const attr_value> val) noexcept {
      if (val.size() > 4) {
        return false;
      }
      for (auto & p : val) {
        if (p.type == attr_tpl) {
          return false;
        }
        if (p.type == attr_str && p.as_str().size() > 12) {
          return false;
        }
      }
      return true;
    }
  }

  void dump(dump_context & ctx, const codegen_template & tpl) noexcept {
    ctx.to.state = ctx.to.state >= code_writer::token ? code_writer::indented : ctx.to.state;
    for (auto & p : tpl) {
      switch (p.break_before) {
      case lbr_t::normal: break;
      case lbr_t::none: ctx.to.clear_break(); break;
      case lbr_t::hard: ctx.to.newline(); break;
      case lbr_t::soft: ctx.to.begin_line(); break;
      }
      switch (p.type) {
      case tpl_part_none:
        ctx.to.write("## ERROR - Empty template part ##", st::error);
        break;
      case tpl_verbatim: {
        auto g = ctx.to.enter(tpl_lines);
        ctx.to.write_with_line_breaks(p.content, st::light_gray);
        break;
      }
      case tpl_newline:
        ctx.to.queue_empty_line();
        break;
      case tpl_expr: {
        auto g = ctx.to.enter(tpl_lines);
        ctx.to.write("%(", st::yellow.with_bold());
        dump(ctx, *p.subst_expr);
        ctx.to.write(")", st::yellow.with_bold());
        break;
      }
      case tpl_if:
      case tpl_let:
      case tpl_set:
      case tpl_foreach:
      case tpl_with_writer_settings:
      case tpl_indent:
      case tpl_indent_inner:
        dump_stmt(ctx, p);
        ctx.to.clear_empty_line();
        break;
      }
      switch (p.break_after) {
      case lbr_t::normal: break;
      case lbr_t::none:
        ctx.to.clear_break(); break;
      case lbr_t::hard: ctx.to.newline(); break;
      case lbr_t::soft: ctx.to.begin_line(); break;
      }
    }
  }

  void dump(dump_context & ctx, const wrapped_node_set & value) noexcept {
    auto s = value ? (ctx.set_color.empty() ? st::cyan.with_bold() : ctx.set_color) : st::red;
    if (ctx.set_style == node_set_ctor) {
      ctx.to.write("Set[", s.with_bold());
    } else {
      ctx.enter_precedence(4U);
    }
    bool is_first = true;
    for (auto & t : value.iterate(ctx.mod)) {
      if (!is_first) {
        ctx.to.write(ctx.set_style == node_set_ctor ? ", " : "|");
      }
      ctx.to.write(t.name, s);
      is_first = false;
    }
    if (ctx.set_style == node_set_ctor) {
      ctx.to.write("]", s.with_bold());
    } else {
      ctx.exit_precedence(4U);
    }
  }

  void dump(dump_context & ctx, const attr_value & value) noexcept {
    switch (value.type) {
    case attr_type_none:
      ctx.to.write("*NoneValue*", st::error);
      return;
    case attr_bool: {
      bool b = value.as_bool();
      ctx.to.write(b ? "True" : "False", b ? st::bright_green : st::bright_red);
      return;
    }
    case attr_int: {
      i64_t n = value.as_int();
      ctx.to.write(strings::format("{}", n), n < 0 ? st::bright_red : (n == 0 ? st::light_gray : st::white));
      return;
    }
    case attr_enum:
      ctx.to.write(value.enum_val.def->get_name(value.enum_val.value), st::bright_green);
      return;
    case attr_term:
    case attr_term_member:
    case attr_value_type:
    case attr_attr_def:
    case attr_enum_def:
    case attr_gen_file_set:
    case attr_struct_def:
    case attr_rule:
    case attr_nonterminal:
    case attr_fn:
      ctx.to.write(value.as_node().name, node_style(value));
      return;
    case attr_term_set:
    case attr_val_type_set:
      dump(ctx, value.as_node_set());
      return;
    case attr_str:
      ctx.to.write("\"", st::bold);
      ++ctx.to.indent;
      ctx.to.write_with_line_breaks(value.as_str(), st::light_gray);
      --ctx.to.indent;
      ctx.to.write("\"", st::bold);
      return;
    case attr_id:
      ctx.to.write("'", st::bright_yellow);
      ctx.to.write(value.id.name.pkg, st::bright_cyan);
      ctx.to.write(":", st::bright_yellow);
      ctx.to.write(value.id.name.name, st::bright_cyan.with_bold());
      if (!value.id.resolved) {
        ctx.to.write("[UNRES]", st::bright_red);
      }
      return;
    case attr_tpl: {
      dump(ctx, value.as_tpl());
      return;
    }
    case attr_tuple: {
      ctx.to.write("(", st::bright_yellow.with_bold());
      bool is_short = is_short_tuple(value.as_tuple());
      if (!is_short) {
        ++ctx.to.indent;
      }
      bool is_first = true;
      for (auto & p : value.as_tuple()) {
        if (!is_short) {
          ctx.to.begin_line();
        } else if (!is_first) {
          ctx.to.write(" ");
        }
        is_first = false;
        dump(ctx, p);
      }
      if (!is_short) {
        --ctx.to.indent;
      }
      ctx.to.write(")", st::bright_yellow.with_bold());
      return;
    }
    case attr_map: {
      ctx.to.write("Map{", st::bright_yellow.with_bold());
      bool is_first = true;
      for(auto & p : *value.map) {
        if (!is_first) {
          ctx.to.write(", ");
        }
        is_first = false;
        ctx.to.write(p.first, st::bold);
        ctx.to.write(": ");
        dump(ctx, p.second);
      }
      ctx.to.write("}", st::bright_yellow.with_bold());
      return;
    }
    case attr_struct: {
      ctx.to.write("(", st::bright_yellow.with_bold());
      bool is_first = true;
      for(auto & f : value.struct_val.def->fields) {
        if (!is_first) {
          ctx.to.write(", ");
        }
        is_first = false;
        ctx.to.write(f, st::bold);
        ctx.to.write(" = ");
        dump(ctx, value.struct_val.at(f));
      }
      ctx.to.write(")", st::bright_yellow.with_bold());
      return;
    }
    }
  }

  void dump(dump_context & ctx, const attrs & attrs, bool space) noexcept {
    for (auto & p : attrs) {
      ctx.to.write(space ? " @" : "@", st::bright_magenta.with_bold());
      ctx.to.write(p.first, st::bright_magenta);
      ctx.to.write(" ");
      {
        auto g = ctx.to.begin_indent();
        dump(ctx, p.second);
      }
      ctx.to.line_break();
    }
  }

  void dump(dump_context & ctx, const term & term) noexcept {
    comment_writer cw{ctx.to, hash_comment};
    cw.write_doc_comment(term.comment, {
      .title.prefix = s::format("Term #{} - ", term.index),
    });
    ctx.to.clear_section_break();
    ctx.to.begin_line();
    ctx.to.write("Term ", st::green);
    ctx.to.write(term.name, st::bright_green.with_bold());
    ctx.to.write("(", st::white.with_bold());
    {
      auto g = ctx.to.begin_indent();
      bool is_first = true;
      for (auto op : term.operands()) {
        cw.reset();
        if (!is_first) {
          ctx.to.write(",");
          ctx.to.line_break();
        }
        is_first = false;
        cw.write_doc_comment(op->comment);
        ctx.to.clear_section_break();
        ctx.to.write_formatted("{#bright_cyan}{}{/} {#bright_yellow,bold}{}{/}", op->type->name, op->name);
        {
          auto g = ctx.to.begin_indent();
          auto g2 = ctx.to.descend_guarded(1);
          dump(ctx, op->attrs, true);
          ctx.to.clear_break();
        }
      }
    }
    ctx.to.write(") {", st::white.with_bold());
    ctx.to.line_break();
    {
      auto g = ctx.to.descend_guarded();
      for (auto op : term.data()) {
        cw.reset();
        cw.write_doc_comment(op->comment);
        ctx.to.write_line_formatted("{#bright_blue}Data{/} {#bright_cyan}{}{/} {#yellow,bold}{}{/};", op->type->name, op->name);
      }
      dump(ctx, term.attrs);
    }
    ctx.to.write_line("}", st::white.with_bold());
  }

  void dump_context::write_header(const char * J_NOT_NULL title, strings::style s) noexcept {
    to.begin_line();
    to.write(" ", s);
    to.write(title, s);
    to.write(" ", s);
    to.queue_empty_line();
  }
  void dump_context::write_index(u32_t index, u8_t pad, strings::style s) noexcept {
    s::string idx_str = s::format("#{}: ", index);
    to.write(s::format("{indent}", max(0, pad + 3 - idx_str.size())), s);
    to.write(idx_str, s);
  }
  void dump_context::write_number(i64_t num, strings::style s) noexcept {
    to.write(s::format("{}", num), s);
  }

  u8_t dump_context::enter_precedence(u8_t prec) noexcept {
    if (precedence > prec) {
      to.write("(", st::light_gray);
    }
    auto result = precedence;
    precedence = prec;
    return result;
  }

  void dump_context::exit_precedence(u8_t prec) noexcept {
    if (precedence < prec) {
      to.write(")", st::light_gray);
    }
    precedence = prec;
  }


  namespace {
    const s::style placeholder_styles[]{
      st::bright_red,
      st::bright_green,
      st::bright_magenta,
      st::bright_yellow,
      st::bright_cyan,
      st::bright_blue,
    };
  }

  [[nodiscard]] s::style get_placeholder_style(u8_t index) noexcept {
    return placeholder_styles[index % J_ARRAY_SIZE(placeholder_styles)];
  }
  void dump_placeholder(dump_context & ctx, u8_t index, bool is_seq, const nonterminal * nt) noexcept {
    s::style s = get_placeholder_style(index);
    ctx.to.write(is_seq ? "...$" : "$", s);
    if (nt && nt->num_fields() >= index) {
      ctx.to.write(nt->field_name(index - 1), st::bright_cyan.with_bold());
    } else {
      ctx.to.write(s::format("{}", index), s.with_bold());
    }
  }


  struct dump_context_str_impl final {
    mem::shared_ptr<streams::string_sink> str;
    s::formatted_sink to;
    code_writer wr;

    dump_context_str_impl()
      : str(j::mem::make_shared<streams::string_sink>()),
        to(j::mem::static_pointer_cast<streams::sink>(str)),
        wr(&to)
    {
    }
  };

  dump_context_str::dump_context_str(module * J_NOT_NULL mod)
    : dump_context_str(::new dump_context_str_impl, mod)
  {
  }
  dump_context_str::~dump_context_str() {
    ::delete impl;
  }

  [[nodiscard]] strings::string dump_context_str::build() noexcept {
    impl->to.flush();
    return static_cast<s::string &&>(impl->str->string);
  }

  dump_context_str::dump_context_str(dump_context_str_impl * J_NOT_NULL impl, module * J_NOT_NULL mod)
    : dump_context(impl->wr, mod),
      impl(impl)
  {
  }
}
