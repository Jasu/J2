#include "graphviz_dump.hpp"
#include "algo/quicksort.hpp"
#include "containers/trivial_array.hpp"
#include "meta/module.hpp"
#include "meta/rules/pat_set.hpp"
#include "strings/escape.hpp"
#include "logging/global.hpp"
#include "strings/string_set.hpp"
#include "graphviz/table.hpp"
#include "meta/rules/grammar.hpp"
#include "meta/rules/dump.hpp"
#include "containers/hash_map.hpp"
#include "meta/rules/nonterminal.hpp"
#include "files/ofile.hpp"
#include "graphviz/edge.hpp"
#include "strings/format.hpp"
#include "graphviz/graph.hpp"
#include "graphviz/node.hpp"
#include "graphviz/serialization.hpp"
#include "mem/shared_ptr.hpp"

namespace j::meta::inline rules {
  namespace g = graphviz;
  namespace a = g::attributes;
  namespace s = strings;

  namespace {
    const char * const br_left = "<br align=\"left\"/>";
    const s::escape_table html_tbl{
      [(u8_t)'<'] = "&lt;",
      [(u8_t)'>'] = "&gt;",
      [(u8_t)'"'] = "&quot;",
      [(u8_t)'&'] = "&amp;",
      [(u8_t)'\n'] = br_left,
    };
    J_A(NI,NODISC,ND) inline s::string html_escape(s::const_string_view in) {
      return escape(in, html_tbl, j::strlen(br_left));
    }

    inline const g::table_style s_title{{}, "#FFFFFF", 10.0f, g::align_left, g::valign_middle, true, false, 1U};

    enum act_type {
      a_shift,
      a_shift_reduce,
      a_reduce,
      a_rewrite,
      a_erase,
      a_recovery,
      a_rewrite_tail,
    };

    const g::table_style act_styles[]{
      {"#FFE090", "#000000", 10.0f, g::align_left, g::valign_top},
      {"#F000F0", "#FFFF68", 10.0f, g::align_left, g::valign_top, true},
      {"#FFFF00", "#000000", 10.0f, g::align_left, g::valign_top},
      {"#FFFF00", "#800000", 10.0f, g::align_left, g::valign_top},
      {"#F08040", "#FFFF68", 10.0f, g::align_left, g::valign_top, true},
      {"#E0E0E0", "#444444", 9.0f, g::align_left, g::valign_top},
      {"#FF80FF", "#442044", 10.0f, g::align_left, g::valign_top, true},
    };

    [[nodiscard]] inline act_type get_act_type(const action & act) noexcept {
      if (act.is_recovery) {
        return a_recovery;
      }
      switch (act.type) {
      case action_type::none: J_UNREACHABLE();
      case action_type::shift: return a_shift;
      case action_type::shift_reduce: return a_shift_reduce;
      case action_type::reduce: {
        act_type t = a_shift;
        for (auto & g : *act.reduce) {
          for (auto & r : g) {
            switch (r.type) {
            case reduction_type::change_type:
            case reduction_type::stmt:
            case reduction_type::reduce:
            case reduction_type::reduce_copy:
              return a_reduce;
            case reduction_type::rewrite:
              t = (t == a_shift || t == a_rewrite) ? a_rewrite : a_reduce;
              break;
            case reduction_type::pop:
            case reduction_type::truncate:
            case reduction_type::erase:
              t = (t == a_shift || t == a_erase) ? a_erase : a_reduce;
              break;
            }
          }
        }
        return t;
      }
      }
    }

    enum lalr_row_type {
      shift,
      shift_default,
      shift_exit,
      goto_,
      goto_default,
      reduce,
      reduce_default,
      reduce_zero,
      shift_reduce,
      rewrite,
      erase,
    };

    const g::table_style st_index = {"#FFFFA0", "#400000", 9.0f, g::align_center, g::valign_middle, true};
    const g::table_style st_table = {"#7777FF", "#000000", 8.0f, g::align_left, g::valign_top};
    const g::table_style st_default = {"#7777FF", "#000000", 8.0f, g::align_left, g::valign_top};
    const g::table_style st_goto = {"#F8A6F8", "#000000", 9.0f, g::align_left};
    const g::table_style st_def_shift = {"#D8D8C0", "#000000", 7.0f, g::align_center, g::valign_top, false, false, 1};
    const g::table_style st_def_goto = {"#E088C8", "#000000", 7.0f, g::align_center, g::valign_top, false, false, 1};
    const g::table_style st_state = {"#EFEFFF", "#404040", 8.0f, g::align_left};

    struct J_TYPE_HIDDEN lalr_state final {
      pat_set_pp set{};
      s::string name{};
      g::node * node = nullptr;
      hash_set<lalr_state*> from{};
      bool has_non_default_use = false;
      i32_t order = I32_MAX;
      void set_order(i32_t o) noexcept {
        if (order > o) {
          order = o;
          for (auto f : from) {
            f->set_order(o);
          }
        }
      }

      bool is_reduce() const noexcept {
        return set->unambiguous_action.is_reduce();
      }
    };

    struct J_TYPE_HIDDEN context final {
      grammar & grammar;
      pat_sets sets;
      g::graph root_graph;
      trivial_array<lalr_state> states{};
      g::graph * init_graph = nullptr;
      g::graph * default_graph = nullptr;
      g::graph * reduce_graph = nullptr;

      [[nodiscard]] bool is_default_action(terminal t, const action & act) const noexcept {
        auto a = states[0].set->actions.maybe_at(t);
        return a && *a == act;
      }

      [[nodiscard]] bool is_default_goto(nt_p J_NOT_NULL nt, pat_set_pp to) const noexcept {
        auto g = states[0].set->gotos.maybe_at(nt);
        return g && *g == to;
      }
    };

    [[nodiscard]] s::string get_reduce_title(context & ctx, const action & act) {
      s::string result;
      for (auto & r : *act.reduce) {
        if (act.type == action_type::shift_reduce) {
          result += "Shift+";
        }
        result += dump_str(ctx.grammar.mod, r);
        if (act.reduce->size() > 1) {
          result += "\n";
        }
      }
      return html_escape(result);
    }

    [[nodiscard]] g::table_column get_action_column(context & ctx, const action & act) noexcept {
      s::string to_idx;
      bool is_reduce = act.is_reduce();
      if (!is_reduce) {
        to_idx = s::format("{}", act.to->index);
      }
      return {
        is_reduce ? get_reduce_title(ctx, act) : "Shift #" + to_idx,
        act_styles[get_act_type(act)],
        is_reduce ? "t" : "s" + to_idx,
        is_reduce ? "" : "#state" + to_idx,
      };
    }

    g::table_column get_terminal_column(context & ctx, terminal_set terminals, const action & act, bool is_root) noexcept {
      s::string title;
      s::string tooltip;
      bool has_rows = false;
      u32_t max_line_len = is_root ? 60 : U32_MAX;
      u32_t line_len = 0U;
      i32_t max_num = is_root ? I32_MAX : 3U;
      if (ctx.grammar.terminals.any_terminal.is_subset_of(terminals)) {
        title += "Any";
        line_len = 3U;
        max_num--;
        terminals -= ctx.grammar.terminals.any_terminal;
      } else {
        u32_t sz = terminals.size();
        u32_t num_terminals = ctx.grammar.terminals.size;
        if (num_terminals - sz < num_terminals / 3) {
          title = "<b>Not</b> ";
          line_len = 4U;
          terminals = ctx.grammar.terminals.any_terminal_or_exit - terminals;
        }
      }
      for (terminal t : terminals) {
        s::string name = ctx.grammar.terminals.name_of(ctx.grammar.mod, t);
        if (!max_num) {
          title += "...";
        } else if (max_num > 0) {
          if (line_len + 2 + name.size() > max_line_len) {
            has_rows = true;
            title += ",<br align=\"left\"/>";
            line_len = 0U;
          } else if (line_len) {
            title += ", ";
            line_len += 2U;
          }
          title += name == "exit" ? "<b>Exit</b>" : name;
          line_len += name.size();
        }
        if (!is_root) {
          if (tooltip) {
            tooltip += ", ";
          }
          tooltip += name;
        }
        --max_num;
      }
      if (has_rows) {
        title += "<br align=\"left\"/>";

      }
      return {
        static_cast<s::string &&>(title),
        act_styles[get_act_type(act)],
        is_root && !act.is_reduce() ? s::format("s{}l", act.to->index) : "",
        "",
        static_cast<s::string &&>(tooltip),
      };
    }

    lalr_state get_lalr_state_info(context & ctx, const pat_set_pp & from) {
      lalr_state result{from};
      if (from->index == 0) {
        result.name = "Init";
      } else {
        u32_t cur_len = 0U;
        bool had_lines = false;
        for (auto & p : from->paths().items()) {
          s::string n = dump_str(nullptr, ctx.grammar, p);
          if (cur_len + 3 + n.size() > 50) {
            result.name.push_back('\n');
            cur_len = 0;
            had_lines = true;
          } else if (result.name) {
            result.name += " / ";
            cur_len += 3U;
          }
          cur_len += n.size();
          result.name += n;
        }
        if (had_lines) {
          result.name.push_back('\n');
        } else if (result.name.size() < 30) {
          result.name += "                         ";
        }
      }
      return result;
    }

    g::node * dump_state(context & ctx, const lalr_state & s) {
      g::table tbl{st_default};
      tbl.push_flex_row((g::table_column[]){
          {s::format("#{}", s.set->index), st_index, "t"},
          {html_escape(s.name), s_title}});
      g::graph * g = ctx.default_graph;
      if (s.set->unambiguous_action && !s.set->gotos) {
        tbl.push_flex_row((g::table_column[]){get_action_column(ctx, s.set->unambiguous_action)});
        if (!s.has_non_default_use && s.set->unambiguous_action.is_reduce()) {
          g = ctx.reduce_graph;
        }
      } else {
        const bool is_init = !s.set->index;
        if (is_init) {
          g = ctx.init_graph;
        }
        tbl.set_style(st_default);
        hash_map<red_p, pair<const action *, terminal_set>, reductions_hash> reductions;
        s::string default_shifts = "";
        const u32_t sz = ctx.sets.size();
        pair<u32_t, terminal_set> shifts[sz];
        j::memzero(shifts, sizeof(pair<u32_t, terminal_set>) * sz);
        for (auto & action : s.set->actions) {
          if (action.second.is_reduce()) {
            auto & p = reductions[action.second.reduce];
            p.first = &action.second;
            p.second += action.first;
          } else {
            if (!is_init && ctx.is_default_action(action.first, action.second)) {
              if (default_shifts) {
                default_shifts += ", ";
              }
              default_shifts += ctx.grammar.terminals.name_of(ctx.grammar.mod, action.first);
            } else {
              auto & s = shifts[action.second.to->index];
              s.first = action.second.to->index;
              s.second += action.first;
            }
          }
        }

        algo::quicksort(shifts, &shifts[sz], [&ctx](auto & s) {
          return ctx.states[s.first].order;
        });

        for (auto & r : reductions) {
          tbl.push_row((g::table_column[]){
              get_terminal_column(ctx, r.second.second, *r.second.first, is_init),
              get_action_column(ctx, *r.second.first),
            });
        }
        for (u32_t i = 0U; i < sz; ++i) {
          if (shifts[i].second) {
            auto & act = s.set->actions.at(shifts[i].second.front());
            tbl.push_row((g::table_column[]){
              get_terminal_column(ctx, shifts[i].second, act, is_init),
              get_action_column(ctx, act),
            });
          }
        }

        u32_t nt_sz = ctx.grammar.mod->nonterminal_ptrs.size();
        pair<nt_p, pat_goto> gotos[nt_sz];
        j::memzero(gotos, sizeof(pair<u32_t, nt_p>) * nt_sz);
        s::string default_gotos = "";
        for (auto & g : s.set->gotos) {
          if (!is_init && ctx.is_default_goto(g.first, g.second)) {
            if (default_gotos) {
              default_gotos += ", ";
            }
            default_gotos += g.first->name;
            continue;
          }
          gotos[g.first->index] = pair<nt_p, pat_goto>{g.first, g.second};
        }

        algo::quicksort(gotos, &gotos[nt_sz], [&ctx](auto & s) {
          return s.second ? ctx.states[s.second->index].order : I32_MAX;
        });

        for (u32_t i = 0U; i < nt_sz; ++i) {
          nt_p nt = gotos[i].first;
          pat_goto g = gotos[i].second;
          if (g) {
            const g::table_style & st = g.is_recovery ? act_styles[a_recovery] : st_goto;
            tbl.push_row((g::table_column[]){
                {nt->name, st, nt->name + "l"},
                {
                  s::format("Goto #{}", g->index),
                  st,
                  nt->name,
                  s::format("#state{}", g->index),
                }
              });
          }
        }

        if (!is_init) {
          tbl.push_flex_row((g::table_column[]){{ "Parent", st_state },
                                                { s.set->state.has_multiple_parents ? "Multiple" : !s.set->state.parent ? "None" : s.set->state.parent->name, st_state },
                                                { "Min", st_state },
                                                { s::format("{}", s.set->state.min_operands_left), st_state },
                                                { "Max", st_state },
                                                { s.set->state.max_operands_left == U32_MAX ? "Inf" : s::format("{}", s.set->state.max_operands_left), st_state }});
          if (default_shifts && default_gotos) {
            tbl.push_flex_row((g::table_column[]){
                { "Default shifts", st_def_shift, "", "#state0", default_shifts },
                { "Default gotos", st_def_goto, "", "#state0", default_gotos }});
          } else if (default_shifts) {
            tbl.push_flex_row((g::table_column[]){{ "Default shifts", st_def_shift, "", "#state0", default_shifts }});
          } else if (default_gotos) {
            tbl.push_flex_row((g::table_column[]){{ "Default gotos", st_def_goto, "", "#state0", default_gotos }});
          }
        }
      }
      return g->add_node(
        a::id = strings::format("state{}", s.set->index),
        a::label_is_html,
        a::label = tbl.render());
    }

    void dump_edges(context & ctx, const lalr_state & from) {
      bool handled_to[ctx.sets.size()];
      j::memzero(handled_to, ctx.sets.size());
      for (auto & a : from.set->actions) {
        if (a.second.is_reduce() || (from.set->index && ctx.sets[0]->actions.contains(a.first) && ctx.sets[0]->actions.at(a.first) == a.second)) {
          continue;
        }
        i32_t to_index = a.second.to->index;
        if (handled_to[to_index]) {
          continue;
        }
        handled_to[to_index] = true;

        auto & to = ctx.states[to_index];
        g::node * to_node = to.node;
        if (to_node != from.node) {
          bool back = !to.has_non_default_use && to.set->unambiguous_action.is_reduce();
          s::string from_port = s::format("s{}{}", to_index, back ? "l:w" : ":e");
          ctx.root_graph.add_edge(
            g::endpoint(back ? to_node : from.node, back ? "t:e" : from_port),
            g::endpoint(!back ? to_node : from.node, !back ? "t:w" : from_port),
            a::arrow_direction = back ? a::direction::both : a::direction::no_value,
            a::arrow_head = back ? a::arrow::none : a::arrow::no_value,
            a::arrow_tail = back ? a::arrow::open : a::arrow::no_value
          );
        }
      }

      for (auto & g : from.set->gotos) {
        if (from.set->index && ctx.sets[0]->gotos.contains(g.first) && ctx.sets[0]->gotos.at(g.first) == g.second) {
          continue;
        }
        i32_t to_index = g.second->index;
        auto & to = ctx.states[to_index];
        g::node * to_node = to.node;
        if (to_node != from.node) {
          bool back = !to.has_non_default_use && to.set->unambiguous_action.is_reduce();
          s::string from_port = s::format("{}{}", g.first->name, back ? "l:w" : ":e");
          ctx.root_graph.add_edge(
            g::endpoint(back ? to_node : from.node, back ? "t:e" : from_port),
            g::endpoint(!back ? to_node : from.node, !back ? "t:w" : from_port),
            a::arrow_direction = back ? a::direction::both : a::direction::no_value,
            a::arrow_head = back ? a::arrow::none : a::arrow::no_value,
            a::arrow_tail = back ? a::arrow::open : a::arrow::no_value,
            a::edge_color = "#505000"
          );
        }
      }
    }

    J_A(AI) inline i32_t lalr_state_sort_key(const lalr_state * s) {
      return -s->from.size();
    }
  }

  void graphviz_dump(const files::path & path, grammar & g) {
    pat_sets sets(g);

    g::table tbl{st_table};
    tbl.push_row((g::table_column[]){
        { "Num States", },
        { s::format("{}", sets.size()) },}
      );

    context ctx{
      g,
      sets,
      g::graph{
        a::concentrate,
        a::is_directional,
        a::label = tbl.render(),
        a::label_is_html,
        a::label_align = a::align::top,
        a::label_justify = a::justify::left,
        a::default_node_attributes = g::node_attributes{
          a::shape = a::node_shape::plain,
          a::font_name = "Roboto",
          a::font_size = 10.0f,
        },
        a::rank_direction = a::rank_dir::left_to_right,
        a::new_rank = true,
        a::ranksep = 2.5f,
        a::nodesep = 0.35f,
        // These are used for graph labels (inherited by subgraphs)
        a::font_name = "Roboto",
        a::font_size = 10.0f,
        a::label_justify = a::justify::left,
        a::graph_style = a::style::filled,
      },
      trivial_array<lalr_state>(containers::uninitialized, sets.size()),
    };
    ctx.init_graph = ctx.root_graph.add_subgraph(a::rank = a::rank_type::same);
    ctx.default_graph = ctx.root_graph.add_subgraph();
    ctx.reduce_graph = ctx.root_graph.add_subgraph(a::rank = a::rank_type::same);
    for (auto & s : ctx.sets) {
      ctx.states.initialize_element(get_lalr_state_info(ctx, s));
    }
    for (auto & state : ctx.states) {
      for (auto & a : state.set->actions) {
        if (!a.second.is_reduce()) {
          if (!ctx.is_default_action(a.first, a.second)) {
            ctx.states[a.second.to->index].from.emplace(&state);
            ctx.states[a.second.to->index].has_non_default_use = true;
          }
        }
      }
      for (auto & g : state.set->gotos) {
        if (!ctx.is_default_goto(g.first, g.second)) {
          ctx.states[g.second->index].from.emplace(&state);
          ctx.states[g.second->index].has_non_default_use = true;
        }
      }
    }
    noncopyable_vector<lalr_state*> reductions;
    for (auto & state : ctx.states) {
      if (state.has_non_default_use && state.from.size() > 1 && state.set->unambiguous_action.is_reduce()) {
        reductions.emplace_back(&state);
      }
    }
    algo::quicksort(reductions.begin(), reductions.end(), lalr_state_sort_key);
    i32_t index = 0U;
    for (auto r : reductions) {
      r->set_order(index++);
    }

    for (auto & state : ctx.states) {
      state.node = dump_state(ctx, state);
    }
    for (auto & state : ctx.states) {
      dump_edges(ctx, state);
    }
    g::serialize(ctx.root_graph, j::mem::make_shared<files::ofile>(path));
  }
}
