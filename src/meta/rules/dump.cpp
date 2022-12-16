#include "dump.hpp"
#include "meta/rules/pattern_tree.hpp"
#include "meta/rules/term_expr.hpp"
#include "logging/global.hpp"
#include "meta/module.hpp"
#include "mem/shared_ptr.hpp"
#include "meta/rules/nonterminal.hpp"
#include "meta/rules/grammar.hpp"
#include "strings/styling/default_styles.hpp"
#include "strings/format.hpp"

namespace j::meta::inline rules {
  namespace s = strings;
  namespace st = strings::styles;

  namespace {
    struct J_TYPE_HIDDEN cap final {
      i32_t index = -1;
      nt_p nt = nullptr;
      i32_t sz = -1;
      i32_t arg_indices[8] = {-1,-1,-1,-1,-1,-1,-1,-1};
      J_A(NODISC,AI) inline bool has_consequetive_args(i32_t n) const noexcept {
        for (i32_t i = 0; i < n; ++i) {
          if (arg_indices[i] != i) {
            return false;
          }
        }
        return true;
      }
      J_A(NODISC,AI) inline bool has_args() const noexcept {
        for (i32_t i : arg_indices) {
          if (i >= 0) {
            return true;
          }
        }
        return false;
      }
      J_A(NODISC,AI) inline explicit operator bool() const noexcept {
        return nt || sz >= 0 || index >= 0;
      }

      J_A(NODISC,AI) inline bool operator!() const noexcept {
        return !nt && sz < 0 && index < 0;
      }
    };

    struct J_TYPE_HIDDEN arg_collector final {
      cap * args;
      i32_t *args_ahead;
      i32_t index_counter = 0U;

      void operator()(const te_term_construct & f) {
        if (f.term_type_expr) {
          visit(f.term_type_expr, *this);
        }
        visit(f.args, *this);
      }

      void operator()(const te_prop_read & f) {
        visit(f.val, *this);
      }

      void operator()(const te_global_fn_call & f) {
        visit(f.args, *this);
      }

      void operator()(const te_binop & f) {
        visit(f.lhs, *this);
        visit(f.rhs, *this);
      }

      void operator()(const te_constant &) const noexcept { }

      void operator()(const reduction_arg & arg) {
        using enum arg_type;
        switch (arg.type) {
        case none: case reference: case capture_term_field:
          J_FAIL("Unsupported arg");
        case capture_term:
          args[arg.stack_offset] = {arg.capture_index};
          break;
        case capture_reduction_field: {
          args[arg.stack_offset].nt = arg.debug_nt;
          i32_t i = 0;
          for (auto & f : arg.debug_nt->fields) {
            if (f.offset == arg.field_offset) {
              args[arg.stack_offset].arg_indices[i] = arg.capture_index;
              break;
            }
            ++i;
          }
          break;
        }
        case capture_reduction_copy:
        case capture_reduction_copy_nt:
        case capture_this_reduction_nt:
            args[arg.stack_offset].nt = arg.debug_nt;
          args[arg.stack_offset].index = arg.capture_index;
          break;
        case capture_reduction_copy_sz:
          args[arg.stack_offset].index = arg.capture_index;
          args[arg.stack_offset].sz = arg.size;
          break;
        case select_term_ahead:
          J_ASSERT(arg.stack_offset < 0);
          args_ahead[-1 - arg.stack_offset] = arg.capture_index;
          break;
        }
      }

      void operator()(const te_ternary & f) {
        visit(f.condition, *this);
        visit(f.true_branch, *this);
        visit(f.false_branch, *this);
      }

      void operator()(const te_member_fn_call & f) {
        visit(f.val, *this);
        visit(f.args, *this);
      }
    };

    struct J_TYPE_HIDDEN stack_view final {
      explicit stack_view(const reduction_group & g) noexcept
        : is_open(g.depth() < 0)
      {
        auto extent = g.extent();
        stack_depth = j::max(g.depth(), extent.first);
        ahead_depth = -extent.second;
        arg_collector c{args, args_ahead, 0};
        for (auto & r : g) {
          for (auto & arg : r.args) {
            visit(arg.expr, c);
          }
        }
      }

      bool dump_arg(dump_context & ctx, const cap & arg, bool had_arg) const noexcept {
        if (!arg) {
          ctx.to.write("·", st::bold);
          return false;
        }
        if (had_arg) {
          ctx.to.write(" ");
        }
        if (arg.nt) {
          ctx.to.write(arg.nt->name, st::bright_magenta);
          if (arg.index >= 0) {
            ctx.to.write_formatted(":${}", arg.index + 1);
          }
          if (arg.nt->fields && arg.has_args()) {
            ctx.to.write("(", st::bold);
            i32_t sz = arg.nt->fields.size();
            const i32_t * in = arg.arg_indices;
            if(sz >= 3 && arg.has_consequetive_args(sz)) {
              ctx.to.write_formatted("${}…{}", in[0] + 1, in[sz - 1] + 1);
            } else {
              had_arg = false;
              for (i32_t i = 0; i != sz; ++i) {
                if (in[i] >= 0) {
                  ctx.to.write_formatted(had_arg ? " ${}" : "${}", in[i] + 1);
                  had_arg = true;
                } else {
                  ctx.to.write("·", st::bold);
                  had_arg = false;
                }
              }
            }
            ctx.to.write(")", st::bold);
          }
        } else if (arg.sz >= 0) {
          ctx.to.write("Blob", st::bright_magenta);
          ctx.to.write_formatted("({})", arg.sz);
          if (arg.index >= 0) {
            ctx.to.write_formatted(":${}", arg.index + 1);
          }
        } else {
          ctx.to.write_formatted("${}", arg.index + 1);
        }
        return true;
      }

      void dump(dump_context & ctx) const noexcept {
        if (is_open) {
          ctx.to.write("⋯", st::light_gray);
        } else if (!stack_depth && !ahead_depth) {
          ctx.to.write("𝛆", st::light_gray);
        }
        bool had_arg = false;
        for (i32_t i = stack_depth - 1; i >= 0; --i) {
          had_arg = dump_arg(ctx, args[i], had_arg);
        }

        if (ahead_depth) {
          ctx.to.write(had_arg ? " /" : "/", st::bold);
          had_arg = true;
          for (i32_t i = 0; i < ahead_depth; ++i) {
            if (args_ahead[i] == -1) {
              ctx.to.write("·", st::bold);
            } else {
              ctx.to.write_formatted(had_arg ? " ${}" : "${}", args_ahead[i] + 1);
            }
            had_arg = args_ahead[i] >= 0;
          }
        }
      }

      bool is_open = false;
      i32_t stack_depth = 0;
      i32_t ahead_depth = 0;
      cap args[16]{};
      i32_t args_ahead[16] = {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};
      i32_t index_counter = 0U;
    };
  }

  void dump(dump_context & ctx, const reduction & r) noexcept {
    switch (r.type) {
    case reduction_type::stmt: {
      dump(ctx, r.args[0].expr);
      break;
    }
    case reduction_type::pop:
      ctx.to.write_formatted("{#bright_red}Pop{/} {#bright_yellow,bold}{}{/}", r.length);
      return;
    case reduction_type::reduce_copy: {
      s::string name = r.nt ? r.nt->name : (r.data_type ? r.data_type->name : "");
      ctx.to.write_formatted("{#bright_green}Copy({}){/}", r.length);
      return;
    }
    case reduction_type::reduce:
      ctx.to.write_formatted("{#bright_magenta,bold}{}{/}", r.nt ? r.nt->name : r.data_type ? r.data_type->name : "");
      break;
    case reduction_type::truncate:
      ctx.to.write_formatted("{#bright_red}Trunc{/}{#bold}({}){/}", r.length);
      return;
    case reduction_type::erase:
      ctx.to.write_formatted("{#bright_red}Erase{/}{#bold}({}){/}", r.length);
      return;
    case reduction_type::change_type:
      ctx.to.write_formatted("{#bright_yellow}Type={/}");
      return;
    case reduction_type::rewrite: break;
    }
    if (r.type == reduction_type::reduce && r.args) {
      ctx.to.write("(", st::bold);
    }
    if (r.type == reduction_type::rewrite) {
      i32_t min_rewrite = I32_MAX, max_rewrite = I32_MIN;
      for (auto & arg : r.args) {
        if (arg.rewrite_action == rewrite_action::replace) {
          min_rewrite = j::min(min_rewrite, arg.rewrite_offset);
          max_rewrite = j::max(max_rewrite, arg.rewrite_offset);
        }
      }
      for (i32_t i = max_rewrite; i >= min_rewrite; --i) {
        ctx.to.write(i == -1 && max_rewrite >= 0 ? " /" : " ");
        bool did_write = false;
        for (auto & arg : r.args) {
          if (arg.rewrite_action == rewrite_action::replace && arg.rewrite_offset == i) {
            dump(ctx, arg.expr);
            did_write = true;
            break;
          }
        }
        if (!did_write) {
          ctx.to.write("·", st::bold);
        }
      }
    }
    if (r.type == reduction_type::reduce && r.args) {
      bool is_first = true;
      for (auto & arg : r.args) {
        if (!is_first) {
          ctx.to.write(" ");
        }
        dump(ctx, arg.expr);
        is_first = false;
      }
      ctx.to.write(")", st::bold);
    }
    if (r.type == reduction_type::rewrite && r.nt && r.nt->index) {
      ctx.to.write_formatted("{#bright_yellow} as {#bold}{}{/}{/}", r.nt->name);
    }
  }

  void dump(dump_context & ctx, const reduction_group & g) noexcept {
    stack_view v(g);
    v.dump(ctx);
    ctx.to.write(" ⇛ ", st::bold);

    if (g.predicate) {
      ctx.to.write("[", st::bright_yellow);
      dump(ctx, g.predicate);
      ctx.to.write("] ", st::bright_yellow);
    }
    i32_t sz = g.size();

    if (sz != 1) {
      ctx.to.write("{ ", st::bright_magenta);
    }
    bool is_first = true;
    for (auto & r : g) {
      // if (r.type != reduction_type::pop) {
        if (!is_first) {
          ctx.to.write("; ", st::bold);
        }
        is_first = false;
        dump(ctx, r);
      // }
    }
    if (sz != 1) {
      ctx.to.write(" }", st::bright_magenta);
    }
  }

  void dump(dump_context & ctx, const red_p & r) noexcept {
    u32_t i = 0U;
    for (auto & r : *r) {
      ctx.to.write_formatted("{}: ", i++);
      dump(ctx, r);
      ctx.to.queue_break();
    }
  }

  void dump(dump_context & ctx, const grammar & g, const pat_elem_vec & p) noexcept {
    bool is_first = true;
    for (auto & e : p) {
      if (!is_first) {
        ctx.to.write(" ");
      }
      is_first = false;
      if (e.nt) {
        ctx.to.write(e.nt->name);
      } else {
        ctx.to.write(g.terminals.name_of(g.mod, e.t));
      }
    }
  }
  void dump(dump_context & ctx, const grammar & g, const pat_paths & paths) noexcept {
    ctx.to.queue_break();
    for (auto & p : paths.items()) {
      dump(ctx, g, p);
      ctx.to.queue_break();
    }
    ctx.to.flush_break();
  }

  void dump(dump_context & ctx, const grammar & g, terminal t, s::style style, bool is_md = false) noexcept {
    if (t.is_epsilon()) {
      ctx.to.write("𝜺", style);
      return;
    }
    if (is_md) {
      if (t.is_exit()) {
        ctx.to.write("**Exit**");
        return;
      }
      ctx.to.write("`");
    }
    ctx.to.write(g.terminals.name_of(g.mod, t), style);
    if (is_md) {
      ctx.to.write("`");
    }
  }

  void dump(dump_context & ctx, const grammar & g, const terminal_set & ls, s::style style, bool is_md = false, bool parens = true) noexcept {
    bool is_first = true;
    if (is_md && ls == g.terminals.any_terminal) {
      ctx.to.write("_Any terminal_", style);
      return;
    } else if (is_md && ls == g.terminals.any_terminal_or_exit) {
      ctx.to.write("_Any terminal_ or **Exit**", style);
      return;
    }
    if (parens && !is_md && ls.size() != 1) {
        ctx.to.write("(", style);
    }
    u8_t cur_term = 255;
    for (auto la : ls) {
      u8_t cur_index = la.is_special() ? 255 : la.index;
      if (!is_first && (la.is_special() || cur_term != cur_index)) {
        if (is_md && cur_term != 255) {
          ctx.to.write("`");
        }
        ctx.to.write(is_md ? ", " : "|", style);
      }
      if (la.is_exit()) {
        ctx.to.write(is_md ? "**Exit**" : "Exit", style);
        cur_term = 255;
        continue;
      } else if (la.is_epsilon()) {
        ctx.to.write(is_md ? "**𝜺**" : "𝜺", style);
        cur_term = 255;
        continue;
      }
      if (cur_index != cur_term) {
        cur_term = cur_index;
        if (is_md) {
          ctx.to.write("`");
        }
        ctx.to.write(g.terminals.name_of(g.mod, la), style);
      }
      is_first = false;
    }
    if (is_md && cur_term != 255) {
      ctx.to.write("`", style);
    }
    if (parens && !is_md && ls.size() != 1) {
        ctx.to.write(")", style);
    }
  }

  void dump_first(dump_context & ctx, const grammar & g, const pat_p & p) noexcept {
    ctx.to.write(" (* ", st::bright_cyan);
    dump(ctx, g, p->first, st::bright_cyan, true);
    ctx.to.write(" *)", st::bright_cyan);
  }

  void dump_rr(dump_context & ctx, const grammar & g, const terminal_set & ls, s::style style) noexcept {
    bool is_first = true;
    if (ls.size() != 1) {
        ctx.to.write("(", style);
    }
    u8_t prev = 255;
    for (auto la : ls) {
      u8_t cur = la.index;
      if (cur != prev || la.is_special()) {
        if (!is_first) {
          if (prev != 255) {
            ctx.to.write("(", style);
          }
          ctx.to.write("\"|", style);
        }
        cur = prev;
        is_first = false;
        ctx.to.write("\"", style);
        if (la == exit) {
          ctx.to.write(")", style);
        } else {
          ctx.to.write(g.terminals.name_of(g.mod, la), style);
        }
      }
    }
    if (prev != 255) {
      ctx.to.write("(");
    }
    ctx.to.write("\"", style);
    if (ls.size() != 1) {
      ctx.to.write(")", style);
    }
  }

  void dump(dump_context & ctx, const grammar & g, const mem::shared_ptr<pat> & p) noexcept {
    if (!p) {
      ctx.to.write("NONE", st::error);
      return;
    }
    switch (p->type) {
    case pat_type::reduce:
      J_ASSERT(!p->next);
      dump(ctx, p->reduce);
      return;
    case pat_type::alternation: {
      ctx.to.write("(", st::bright_cyan);
      bool is_first = true;
      for (auto & alt : p->alternates) {
        if (!is_first) {
          ctx.to.queue_break();
          ctx.to.write("|", st::bright_cyan);
        }
        is_first = false;
        dump(ctx, g, alt);
      }
      ctx.to.write(")", st::bright_cyan);
      break;
    }
    case pat_type::terminal:
      if (p->first == terminal_set{exit}) {
        ctx.to.write(")", st::bright_red);
      } else {
        dump(ctx, g, p->first, st::bright_green.with_bold());
      }
      ctx.to.write("(", st::bright_green);
      break;
    case pat_type::nonterminal:
      if (!p->nonterm) {
        ctx.to.write("None", st::error);
        return;
      }
      ctx.to.write(p->nonterm->name, st::bright_magenta.with_bold());
      break;
    }
    if (p->next) {
      dump(ctx, g, p->next);
    }
  }

  void dump(dump_context & ctx, const grammar & g, nt_p nt) noexcept {
    ctx.to.begin_line();
    ctx.to.write("NonTerminal ", st::bright_magenta);
    if (!nt) {
      ctx.to.write("NONE", st::error);
    } else {
      ctx.to.write(nt->name, st::bright_magenta.with_bold());
      ctx.to.write(" = ");
      auto guard = ctx.to.begin_indent();
      if (nt) {
        dump(ctx, g, nt->pat);
      }
    }
    ctx.to.write(";");
    ctx.to.queue_break();
  }

  namespace {
    [[nodiscard]] s::string format_single_red(const reduction & r) noexcept {
      s::string result;
      switch (r.type) {
      case reduction_type::reduce: result = "Reduce"; break;
      case reduction_type::pop: result = "Pop"; break;
      case reduction_type::reduce_copy: result = "Copy"; break;
      case reduction_type::rewrite: result = "Rewrite"; break;
      case reduction_type::truncate: result = "Truncate"; break;
      case reduction_type::erase: result = "Erase"; break;
      case reduction_type::change_type: result = "SetType"; break;
      case reduction_type::stmt: result = "Stmt"; break;
      }
      result = s::format("{}({})", result, r.length);
      return result;
    }

    [[nodiscard]] s::string format_single_pat(const grammar & g, const pat_p & p) noexcept {
      s::string result;
      switch (p->type) {
      case pat_type::alternation: J_FAIL("Alternation");
      case pat_type::terminal:
        result = g.terminals.name_of(g.mod, terminal{(*p->first.begin()).index});
        break;
      case pat_type::nonterminal:
        result = p->nonterm->name;
        break;
      case pat_type::reduce: {
        J_FAIL("Reduce");
      }
      }
      if (p->precedence.prec() != 0) {
        result = s::format("{}[{}]", result, p->precedence.prec());
      }
      return result;
    }

    [[nodiscard]] noncopyable_vector<s::string> format_branch(const grammar & g, const pat_p & p) noexcept {
      noncopyable_vector<s::string> result;
      bool is_first = true;
      if (p->type == pat_type::alternation) {
        u32_t sz = p->alternates.size();
        for (auto & alt : p->alternates) {
          --sz;
          bool is_subresult_first = true;
          for (auto & s : format_branch(g, alt)) {
            if (is_first && is_subresult_first) {
              result.emplace_back((sz ? "─┬─" : "───") + s);
            } else if (is_subresult_first) {
              result.emplace_back((sz ? " ├─" : " └─") + s);
            } else {
              result.emplace_back((sz ? " │ " : "   ") + s);
            }
            is_subresult_first = false;
          }
          is_first = false;
        }
      } else if (p->type == pat_type::reduce) {
        i32_t n_groups = p->reduce->size();
        for (auto & g : *p->reduce) {
          --n_groups;
          i32_t n_reds = g.size();
          bool is_subresult_first = true;
          for (auto & r : g) {
            n_reds--;
            s::string s = format_single_red(r);
            if (is_first && is_subresult_first) {
              result.emplace_back((n_groups || n_reds ? "─┬─" : "───") + s);
            } else if (is_subresult_first) {
              result.emplace_back((n_groups || n_reds ? " ├─" : " └─") + s);
            } else {
              result.emplace_back((n_groups || n_reds ? " │ " : "   ") + s);
            }
            is_subresult_first = false;
          }
          is_first = false;
        }
      } else {
        s::string single = "╼ " + format_single_pat(g, p);
        if (!p->next) {
          result.emplace_back(static_cast<s::string &&>(single));
        } else {
          result = format_branch(g, p->next);
          single += " ╾";
          s::string pad{single.size(), ' '};
          bool is_first = true;
          for (auto & r : result) {
            r = (is_first ? single : pad) + r;
            is_first = false;
          }
        }
      }
      return result;
    }
  }

  void debug_dump_pat(const grammar & g, const pat_p & p) noexcept {
    for (auto & str : format_branch(g, p)) {
      J_DEBUG("{}", str);
    }
  }

  void dump(dump_context & ctx, const grammar & g, const terminal_set & ls) noexcept {
    dump(ctx, g, ls, st::bright_cyan, false, false);
  }
}
