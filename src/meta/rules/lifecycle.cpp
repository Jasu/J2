#include "lifecycle.hpp"
#include "meta/rules/id_resolver.hpp"
#include "meta/rules/dump.hpp"
#include "meta/module.hpp"
#include "meta/rules/grammar.hpp"
#include "meta/rules/rule.hpp"
#include "meta/rules/pattern_tree.hpp"
#include "logging/global.hpp"
#include "meta/rules/reduction.hpp"
#include "exceptions/assert.hpp"
#include "hzd/iterators.hpp"
#include "strings/format.hpp"

namespace s = j::strings;

namespace j::meta::inline rules {
  void dump_te_loc(term_expr * te, bool is_rel) noexcept {
    if (!te) {
      return;
    }
    if (is_rel) {
      J_DEBUG_INL("{#bright_red,bold}[{/}");
    }
    s::string name = "";
    switch (te->type) {
    case term_expr_type::global_fn_call: name = te->global_fn_call.fn_name; break;
    case term_expr_type::member_fn_call: name = te->member_fn_call.fn_name; break;
    case term_expr_type::term_construct: name = "Term ctor"; break;
    case term_expr_type::ternary: name = "If"; break;
    case term_expr_type::binop: name = s::string("Op ") + operator_info[(i32_t)te->binop.type].name; break;
    default:
      break;
    }
    if (name) {
      J_DEBUG_INL("{#bright_yellow,bold}{}{/} {#bright_red}at{/} ", name);
    }
    J_DEBUG_INL(is_rel ? "{#bright_red,bold}{}]{/}" : "{#bright_green}{}{/}", te->loc);
  }

  i32_t len_te_loc(term_expr * te, bool brackets) noexcept {
    if (!te) {
      return 0;
    }
    i32_t len = brackets ? 2 : 0;
    switch (te->type) {
    case term_expr_type::global_fn_call: len += 4 + te->global_fn_call.fn_name.size(); break;
    case term_expr_type::member_fn_call: len += 4 + te->member_fn_call.fn_name.size(); break;
    case term_expr_type::term_construct: len += 4 + 9; break;
    case term_expr_type::ternary: len += 4 + 2; break;
    case term_expr_type::binop: len += 4 + 3 + strlen(operator_info[(i32_t)te->binop.type].name); break;
    default:
      break;
    }
    return len + s::format("{}", te->loc).size();
  }

  i32_t len(const output_stack_val & val) noexcept {
    return val.is_alloc() ? len_te_loc(val.te, false) : s::format("@{}", val.reference.stack_offset).size();
  }

  void dump(const output_stack_val & val, i32_t pad) noexcept {
    if (val.is_alloc()) {
      dump_te_loc(val.te, false);
    } else {
      J_DEBUG_INL("{#bright_yellow,bold}@{}{/}", val.reference.stack_offset);
    }
    J_DEBUG_INL("{indent}", pad - len(val));
  }

  i32_t len(const input_stack_val & val) noexcept {
    return val.field->name.size() + len_te_loc(val.consumed_at, true);
  }
  void dump(const input_stack_val & val, i32_t pad) noexcept {
    J_DEBUG_INL(val.consumed_at ? (val.is_alive() ? "{#error}{}{/}" : "{#light_gray}{}{/}") : "{#bright_green}{}{/}", val.field->name);
    dump_te_loc(val.consumed_at, true);
    J_DEBUG_INL("{indent}", pad - val.field->name.size() - len_te_loc(val.consumed_at, true));
  }

  [[nodiscard]] i32_t title_len(const grammar & g, const lr_state & lr) noexcept {
    i32_t title_max = 0, i = 0;
    for (auto & l : reverse_iterator_view{&lr}) {
      i32_t title_len = s::format("#{}(-{}) ", lr.size() - 1 - i, i).size();
      title_len += l.nt ? l.nt->name.size() : dump_str(g.mod, g, l.term).size();
      title_max = max(title_max, title_len);
      ++i;
    }
    return title_max;
  }

  void dump_title(const grammar & g, const lr_state & lr, i32_t i, i32_t max_len) noexcept {
    i32_t abs = lr.size() - 1 - i;
    s::string abs_str = s::format("{}", abs), rel_str = s::format("{}", i);
    auto & l = lr[abs];
    s::string name = l.nt ? l.nt->name : dump_str(g.mod, g, l.term);
    i32_t len = max_len - abs_str.size() - rel_str.size() - 5 - name.size();
    J_DEBUG_INL(!l.nt
                ? "{#cyan}@{}{/}{#yellow}(-{}){/} {#bright_cyan}{}{/}{indent}"
                : l.nt->num_tracked_fields()
                ? "{#cyan}@{/}{#bold,bright_cyan}{}{/}{#bright_yellow}(-{#bold}{}{/}){/} {#bright_green,bold}{}{/}{indent}"
                : "{#cyan,bold}@{/}{#bright_cyan}{}{/}{#bright_yellow,bold}(-{}){/} {#bright_green,bold}{}{/}{indent}"
                , abs_str, rel_str, name, len);
  }

  void dump_in(const grammar & g, const input_state & in) noexcept {
    i32_t in_maxs[4] = {0,0,0,0};
    for (auto & l : reverse_iterator_view{&in.lr}) {
      for (i32_t i = 0; i < in.size(); ++i) {
        in_maxs[i] = j::max(in_maxs[i], l.fields[i] >= 0 ? len(in[i][l.fields[i]]) : 1);
      }
    }

    i32_t i = 0, title_max = title_len(g, in.lr);
    for (auto & l : reverse_iterator_view{&in.lr}) {
      dump_title(g, in.lr, i, title_max);
      for (i32_t i = 0; i < in.size(); ++i) {
        if (l.fields[i] >= 0) {
          J_DEBUG_INL(" {#bold}|{/} ");
          dump(in[i][l.fields[i]], in_maxs[i]);
        } else {
          J_DEBUG_INL(" {#bold}|{/} {#light_gray}∅{/}{indent}", in_maxs[i] - 1);
        }
      }
      ++i;
      J_DEBUG("");
    }
  }

  void dump_out(const grammar & g, const output_state & in) noexcept {
    i32_t in_maxs[4] = {0,0,0,0};
    for (auto & l : reverse_iterator_view{&in.lr}) {
      for (i32_t i = 0; i < in.size(); ++i) {
        in_maxs[i] = j::max(in_maxs[i], l.fields[i] >= 0 ? len(in[i][l.fields[i]]) : 1);
      }
    }

    i32_t i = 0, title_max = title_len(g, in.lr);
    for (auto & l : reverse_iterator_view{&in.lr}) {
      dump_title(g, in.lr, i, title_max);
      for (i32_t i = 0; i < in.size(); ++i) {
        if (l.fields[i] >= 0) {
          J_DEBUG_INL(" {#bold}|{/} ");
          dump(in[i][l.fields[i]], in_maxs[i]);
        } else {
          J_DEBUG_INL(" {#bold}|{/} {#light_gray}∅{/}{indent}", in_maxs[i] - 1);
        }
      }
      ++i;
      J_DEBUG("");
    }
  }

  void input_stack_val::exit_if(term_expr * J_NOT_NULL te, const input_stack_val & branch0, const input_stack_val & branch1) {
    J_ASSERT(branch0.nt == branch1.nt && branch0.field == branch1.field);
    if (!consumed_at) {
      J_WARNING_IF(!branch0.consumed_at != !branch1.consumed_at, "Value was consumed in one branch only");
      if (branch0.consumed_at || branch1.consumed_at) {
        consumed_at = te;
      }
    }
    if (!consumed_at) {
      if (branch0.uses || branch1.uses) {
        uses.emplace_back(te, false);
      }
    }
  }
  input_state::input_state(const module * J_NOT_NULL mod) noexcept {
    resize(mod->term_stacks.size());
  }

  output_state::output_state(const module * J_NOT_NULL mod) noexcept {
    resize(mod->term_stacks.size());
  }
  void input_state::clear(const module * J_NOT_NULL mod) noexcept {
    static_vector::clear();
    resize(mod->term_stacks.size());
    lr.clear();
  }

  void output_state::clear(const module * J_NOT_NULL mod) noexcept {
    static_vector::clear();
    resize(mod->term_stacks.size());
    lr.clear();
  }

  pat_type_state::pat_type_state(module * J_NOT_NULL m) noexcept
    : input(m),
      output(m)
  {
  }

  void pat_type_state::enter_branch() noexcept {
    for (auto & stack : input) {
      for (auto & entry : stack) {
        entry.enter_branch();
      }
    }
  }
  input_state pat_type_state::exit_branch(const input_state & snapshot) noexcept {
    input_state res = input;
    input = snapshot;
    return res;
  }

  void pat_type_state::exit_if(term_expr * J_NOT_NULL te, const input_state & snap, const input_state & b0, const input_state & b1) {
    input = snap;
    auto i0 = b0.begin(), i1 = b1.begin();
    for (auto & stack : input) {
      auto e0 = i0->begin(), e1 = i1->begin();
      for (auto & entry : stack) {
        entry.exit_if(te, *e0, *e1);
        ++e0, ++e1;
      }
      ++i0, ++i1;
    }
  }

  void pat_type_state::push_back(const pat_p & pat) noexcept {
    if (pat->type == pat_type::nonterminal) {
      auto lrs = input.lr.emplace_back(pat->nonterm);
      if (pat->nonterm->has_lifetime_tracking()) {
        for (auto & field : pat->nonterm->fields) {
          if (field.tracking_index >= 0) {
            i32_t in_index = input[field.tracking_index].size();
            input[field.tracking_index].emplace_back(pat->nonterm, &field);
            lrs->fields[field.tracking_index] = in_index;
          }
        }
      }
    } else {
      J_ASSERT(pat->type == pat_type::terminal && pat->first);
      input.lr.emplace_back(pat->first);
    }
  }

  namespace {
    // output_stack_val parse_ref(const lr_state & lr, term_expr * J_NOT_NULL te) {
    //   const nt_data_field * field = nullptr;
    //   i32_t stack_offset = -1;
    //   switch (te->type) {
    //   case term_expr_type::arg:
    //     J_ASSERT(te->arg.type == arg_type::capture_reduction_field, "Assign to non-field value at {}", te->loc);
    //     J_ASSERT_NOT_NULL(te->arg.debug_nt);
    //     field = &te->arg.debug_nt->field_at_offset(te->arg.field_offset);
    //     stack_offset = te->arg.stack_offset;
    //     break;
    //   case term_expr_type::prop_read:
    //     J_ASSERT(te->prop_read.val->type == term_expr_type::arg, "Assign to field of non-argument at {}", te->loc);
    //     J_ASSERT_NOT_NULL(te->prop_read.val->arg.debug_nt);
    //     stack_offset = te->prop_read.val->arg.stack_offset;
    //     field = &te->prop_read.val->arg.debug_nt->field_at(te->prop_read.prop_name);
    //     break;
    //   default:
    //     J_THROW("Unsupported lhs at {}", te->loc);
    //   }
    //   J_ASSERT(stack_offset >= 0);
    //   if (!field->has_lifetime_tracking) {
    //     return {};
    //   }
    //   return output_stack_val::ref(lr[lr.size() - 1 - stack_offset].fields[field->tracking_index], field->tracking_index, field->type);
    // }

    enum use_result {
      ur_use,
      ur_use_by_ref,
      ur_ignore,
      ur_stmt,
    };

    struct J_TYPE_HIDDEN consumes_t final : static_vector<output_stack_val, 4> {
      output_stack_val move_dst;
      output_stack_val move_src;
    };

    struct J_TYPE_HIDDEN te_visitor final {
      pat_type_state & state;
      expr_scope & scope;

      output_stack_val operator()(reduction_arg & arg, use_result, term_expr * J_NOT_NULL te) const {
        switch (arg.type) {
        case arg_type::capture_term_field:
        case arg_type::reference:
        case arg_type::none: J_FAIL("Invalid arg");
        case arg_type::capture_reduction_field: {
          J_ASSERT_NOT_NULL(arg.debug_nt);
          if (!arg.value_type->has_lifetime_tracking()) {
            return {};
          }
          const nt_data_field * field = nullptr;
          for (auto & f : arg.debug_nt->fields) {
            if (arg.field_offset == f.offset) {
              field = &f;
              break;
            }
          }
          auto & nt = state.input.lr[state.input.lr.size() - 1 - arg.stack_offset];
          return output_stack_val::ref(nt.fields[field->tracking_index], field->tracking_index, field->type, te);
        }
        default:
          break;
        }
        return {};
      }

      inline void visit_fn_arg(const te_function_arg & arg, term_expr * J_NOT_NULL val, term_expr * J_NOT_NULL fn, output_stack_val & result, consumes_t & consumes, use_result ur) {
        bool is_written = arg.is_written() || arg.consumes_arg() || (arg.returns_arg() && ur == ur_use_by_ref);
        output_stack_val r = visit_with_expr(val, *this, is_written ? ur_use_by_ref : ur_use);
        if (r) {
          if (arg.returns_arg()) {
            result = r.wrap(fn);
          } else if (arg.is_move_src()) {
            add_use(fn, r, ur == ur_use_by_ref);
            consumes.move_src = r;
          } else if (arg.is_move_dst()) {
            consumes.move_dst = r;
          } else if (arg.consumes_arg()) {
            consumes.push_back(r);
          } else {
            add_use(fn, r, ur == ur_use_by_ref);
          }
        }
      }

      void apply_consumes(consumes_t & c, term_expr * J_NOT_NULL te, use_result ur) {
        for (auto & v : c) {
          if (v.is_ref()) {
            auto & src = state.input[v.reference];
            J_REQUIRE(src.is_alive(), "Double consume of {}.{} at {} (released at {})", src.nt->name, src.field->name, te->loc, src.consumed_at->loc);
            src.consume(te, ur == ur_use || ur == ur_use_by_ref);
            if (ur == ur_stmt) {
              src.consume_inserted = true;
            }
          }
        }
        if (c.move_src || c.move_dst) {
          J_ASSERT(c.move_src && c.move_dst);
          J_ASSERT(c.move_dst.is_ref());
          state.output[c.move_dst.reference] = c.move_src;
          if (c.move_src.is_ref()) {
            state.input[c.move_src.reference].move(c.move_dst.reference);
          }
        }
      }

      output_stack_val operator()(te_binop & b, use_result ur, term_expr * J_NOT_NULL te) {
        te_type types[]{b.lhs->get_type(scope), b.rhs->get_type(scope)};
        J_ASSERT(types[0].is_val_type());
        const te_function & fn = types[0].value_type->operators.get_operator(b.type, span(types, 2));
        output_stack_val result;
        consumes_t consumes;
        visit_fn_arg(fn.args[0], b.lhs, te, result, consumes, ur);
        visit_fn_arg(fn.args[1], b.rhs, te, result, consumes, ur);
        apply_consumes(consumes, te, ur);
        return fn.acquires_result() ? output_stack_val::alloc(te, te_type(fn.return_type)) : result;
      }

      output_stack_val operator()(te_constant &, use_result, term_expr * J_NOT_NULL) const noexcept { return {}; }

      output_stack_val operator()(te_term_construct & f, use_result, term_expr * J_NOT_NULL te) {
        // J_ASSERT(!is_consume(at));
        if (!f.term) {
          visit_te(f.term_type_expr, ur_use);
        }
        for (term_expr * arg : f.args) {
          add_use(te, visit_te(arg, ur_use), false);
        }
        return {};
      }

      void add_use(term_expr * J_NOT_NULL te, const output_stack_val & use, bool by_ref) {
        if (use.is_ref()) {
          state.input[use.reference].use(te, by_ref);
        }
      }

      J_A(AI,ND) inline output_stack_val visit_te(term_expr * J_NOT_NULL te, use_result ur) {
        return visit_with_expr(te, *this, ur);
      }

      output_stack_val operator()(te_prop_read & f, use_result, term_expr * J_NOT_NULL te) {
        te_type t = f.val->get_type(scope);
        visit_te(f.val, ur_use);
        if (t.is_any_nt()) {
          J_ASSERT(t.is_nt());
          const nt_data_field * field = &t.nt->field_at(f.prop_name);
          if (!field->has_lifetime_tracking) {
            return {};
          }
          J_ASSERT(f.val->type == term_expr_type::arg && (f.val->arg.type == arg_type::capture_reduction_copy_nt || f.val->arg.type == arg_type::capture_this_reduction_nt));
          auto & nt = state.input.lr[state.input.lr.size() - 1 - f.val->arg.stack_offset];
          return output_stack_val::ref(nt.fields[field->tracking_index], field->tracking_index, field->type, te);
        }
        return {};
      }

      output_stack_val operator()(te_member_fn_call & f, use_result ur, term_expr * J_NOT_NULL te) {
        const te_function & fn = f.find_overload(scope);
        const te_function_arg * arg = fn.args.begin();
        output_stack_val result;
        consumes_t consumes;
        visit_fn_arg(*arg++, f.val, te, result, consumes, ur);
        for (term_expr * val : f.args) {
          visit_fn_arg(*arg, val, te, result, consumes, ur);
          if (!arg->is_variadic()) {
            ++arg;
          }
        }
        apply_consumes(consumes, te, ur);
        return fn.acquires_result() ? output_stack_val::alloc(te, te_type(fn.return_type)) : result;
      }

      output_stack_val operator()(te_global_fn_call & f, use_result ur, term_expr * J_NOT_NULL te) {
        const te_function & fn = f.find_overload(scope);
        const te_function_arg * arg = fn.args.begin();
        output_stack_val result;
        consumes_t consumes;
        for (term_expr * val : f.args) {
          visit_fn_arg(*arg, val, te, result, consumes, ur);
          if (!arg->is_variadic()) {
            ++arg;
          }
        }
        apply_consumes(consumes, te, ur);
        return fn.acquires_result() ? output_stack_val::alloc(te, te_type(fn.return_type)) : result;
      }

      output_stack_val operator()(te_ternary & f, use_result ur, term_expr * J_NOT_NULL te) {
        visit_te(f.condition, ur_use);
        auto snap = state.snapshot();
        state.enter_branch();
        output_stack_val v0 = visit_te(f.true_branch, ur);
        auto b0 = state.exit_branch(snap);
        output_stack_val v1 = visit_te(f.false_branch, ur);
        auto b1 = state.exit_branch(snap);
        state.exit_if(te, snap, b0, b1);
        J_WARNING_IF(!v0 != !v1, "Ternary returns stack value on other side but not other");
        J_WARNING_IF(v0.is_alloc() != v1.is_alloc(), "Ternary returns allocated stack value on other side but not other");
        return v0;
      }
    };

    struct J_TYPE_HIDDEN debug_name final {
      i32_t index = -1;
      s::const_string_view name = "";
      source_location loc{};
      J_A(AI,NODISC) inline explicit operator bool() const noexcept { return index >= 0; }

      inline void enter(i32_t index, s::const_string_view name, source_location loc) noexcept {
        this->index = index;
        this->name = name;
        this->loc = loc;
      }

      inline void reset() noexcept {
        index = -1;
        name = "";
        loc = {};
      }

      inline void dump(const char * J_NOT_NULL type, bool print_idx = true) const noexcept {
        J_ASSERT(index >= 0);
        if (name) {
          J_DEBUG_INL("{}", name);
        } else if (print_idx) {
          J_DEBUG_INL("{}#{}", type, index);
        } else {
          J_DEBUG_INL("{}", type);
        }
        if (loc) {
          J_DEBUG_INL("[{}]", loc);
        }
      }
    };

    struct J_TYPE_HIDDEN reduction_resolver final {
      pat_tree & tree;
      grammar & g;
      expr_scope & scope;
      const reductions & reds;
      pat_layer layer = pat_layer::main;
      i8_t precedence = 0;
      i32_t index = 0;
      bool dbg_enabled = false;
      debug_name dbg_rule{};
      debug_name dbg_grp{};
      debug_name dbg_red{};

      inline reduction_resolver(grammar & g, expr_scope & scope, rule * J_NOT_NULL r) noexcept
        : tree(*r->match),
        g(g),
        scope(scope),
        reds(r->reds),
        layer(r->layer),
        precedence(r->precedence),
        dbg_enabled(r->debug),
        dbg_rule{0, r->name, r->loc}
      {
      }

      template<typename... Ts>
      void debug(const char * J_NOT_NULL tpl, const Ts & ... ts) const noexcept {
        if (!dbg_enabled) {
          return;
        }
        dbg_rule.dump("Rule", false);
        if (dbg_grp) {
          J_DEBUG_INL("/");
          dbg_grp.dump("Grp");
          if (dbg_red) {
            J_DEBUG_INL("/");
            dbg_red.dump("Red");
          }
        }
        J_DEBUG_INL(": ");
        J_DEBUG(tpl, ts...);
      }

      void visit(pat_p pat, pat_type_state state) {
        if (pat) {
          for (;;) {
            J_ASSERT(pat && pat->type != pat_type::reduce);
            if (pat->type == pat_type::alternation) {
              for (auto & a : pat->alternates) {
                visit(a, state);
              }
              return;
            }
            state.push_back(pat);
            if (!pat->next) {
              break;
            }
            pat = pat->next;
          }
        }
        pat_branch * branch = nullptr;
        if (pat) {
          for (pat_branch & br : tree.branches) {
            if (pat ? br.tail == &pat->next : !br.tail) {
              branch = &br;
              break;
            }
          }
        } else {
          J_ASSERT(tree.branches.size() == 1);
          branch = tree.branches.begin();
          J_ASSERT(branch->reduction_depth == 0);
        }
        J_ASSERT(branch);

        reductions breds = reds;
        breds.resolve(tree, *branch, scope);
        pat_prec_type precedence_type = pat_prec_type::rewrite;
        i32_t i = 0;
        for (auto & group : breds) {
          dbg_grp.enter(i, group.name, group.loc);
          pat_type_state group_state = state;
          i32_t j = 0;
          if (dbg_enabled) {
            debug("{#green,bold,white_bg} IN:{/}");
            dump_in(g.mod, group_state.input);
          }
          for (reduction & red : group) {
            dbg_red.enter(j, "", red.loc);
            precedence_type = red.type == reduction_type::rewrite ? precedence_type : pat_prec_type::reduce;
            compute_reduction_output(red, group_state, *branch);

            if (dbg_enabled) {
              debug("{#green,bold,white_bg} OUT:{/}");
              dump_out(g.mod, group_state.output);
            }
            apply_reduction_output(red, group_state);
            ++j;
          }
          dbg_red.reset();
          ++i;
        }
        dbg_grp.reset();
        pat_precedence prec = {layer, (i8_t)precedence, precedence_type, index};
        ++index;
        branch->set_reduction(g.add_reduction_raw(static_cast<reductions &&>(breds)), prec);
      }

      void copy_output_state(output_state & to, pat_type_state & from, i32_t depth) {
        to.clear(g.mod);
        J_ASSERT_RANGE(0, depth, from.input.lr.size() + 1);
        for (i32_t i = 0; i < depth; ++i) {
          auto & lr = from.input.lr[i];
          to.lr.emplace_back(lr);
          if (!lr.nt) {
            continue;
          }
          for (i32_t j = 0; j < to.size(); ++j) {
            i32_t offset = from.input.lr[i].fields[j];
            if (offset < 0) {
              continue;
            }
            to[j].emplace_back(output_stack_val::ref(offset, j, from.input.lr[i].nt->field_by_tracking_index(j).type));
          }
        }
      }

      void apply_reduction_output(reduction & red, pat_type_state & state) {
        i32_t offset = 0;
        for (i32_t i = state.input.lr.size() - 1, end = state.output.lr.size() - 1; i >= 0; --i, ++offset) {
          debug("{} frame #{}", i > end ? "Releasing" : "Applying", offset);
          if (i > end) {
            release_frame(offset, red, state.input, red.loc);
          } else {
            apply_frame_consumes(offset, red, state.input);
          }
        }
        input_state new_input(g.mod);
        for (auto & lr : state.output.lr) {
          if (!lr.nt) {
            continue;
          }
          for (i32_t i = 0; i < new_input.size(); ++i) {
            if (lr.fields[i] < 0) {
              continue;
            }
            auto & out = state.output[i][lr.fields[i]];
            if (out.is_ref()) {
              new_input[i].push_back(state.input[out.reference]);
            } else {
              new_input[i].emplace_back(lr.nt, &lr.nt->field_by_tracking_index(i));
            }
          }
        }
      }

      void compute_reduction_output(reduction & red, pat_type_state & state, const pat_branch & branch) {
        const i32_t depth = red.length < 0 ? branch.reduction_depth + 1 + red.length : red.length;
        const i32_t stack_base = state.input.lr.size() - depth; //branch.reduction_depth - depth;
        J_ASSERT(stack_base >= 0);
        const i32_t nargs = red.args.size();
        output_stack_val output[nargs];
        copy_output_state(state.output, state, stack_base);
        use_result use = red.type == reduction_type::stmt ? ur_stmt : ur_use;
        te_visitor visitor{state, scope};
        for (i32_t i = 0; i < nargs; ++i) {
          J_ASSERT_NOT_NULL(red.args[i].expr);
          output[i] = visitor.visit_te(red.args[i].expr, use);
        }

        if (red.type == reduction_type::reduce) {
          nt_p nt = red.nt;
          J_ASSERT(nt);
          output_stack_val * out = output;
          auto lr = state.output.lr.emplace_back(nt);
          for (auto & f : nt->fields) {
            if (f.has_lifetime_tracking) {
              J_ASSERT(*out);
              lr->fields[f.tracking_index] = state.output[f.tracking_index].size();
              state.output[f.tracking_index].push_back(*out);
              if (out->is_ref()) {
                state.input[out->reference].move(lr->fields[f.tracking_index], f.tracking_index);
              }
            }
            ++out;
          }
        } else if (red.type == reduction_type::reduce_copy) {
          J_ASSERT(red.args.size() == 1);
          auto & arg = red.args[0].expr;
          J_ASSERT(arg && arg->type == term_expr_type::arg && arg->arg.is_nonterminal());
          auto & lr = state.input.lr[state.input.lr.size() - 1 - arg->arg.stack_offset];
          J_ASSERT(lr.nt);
          auto lrs = state.output.lr.emplace_back(lr.nt);
          for (i32_t i = 0; i < state.output.size(); ++i) {
            if (lr.fields[i] >= 0) {
              lrs->fields[i] = state.output[i].size();
              state.output[i].push_back(output_stack_val::ref(lrs->fields[i], i, lr.nt->field_by_tracking_index(i).type));
              state.input[i][lr.fields[i]].move(lrs->fields[i], i);
            }
          }
        }
      }

      void apply_frame_consumes(i32_t idx, reduction & red, input_state & in) {
        auto & lr = in.lr[in.lr.size() - 1 - idx];
        for (i32_t i = 0; i < in.size(); ++i) {
          if (lr.fields[i] < 0) {
            continue;
          }
          auto & f = in[i][lr.fields[i]];
          if (f.moved_to) {
            continue;
          }
          if (f.consumed_at) {
            for (auto & use : f.uses) {
              if (use.te == f.consumed_at) {
                continue;
              }
              red.statements.emplace_back(use.te, use.te, use.by_ref);
            }
          }
          f.uses.clear();
          if (!f.consume_inserted && f.consumed_at) {
            red.statements.emplace_back(f.consumed_at, f.consumed_at_used ? f.consumed_at : nullptr, false);
          f.consume_inserted = true;
          }
        }
      }

      void release_frame(i32_t idx, reduction & red, input_state & in, const source_location & loc) {
        auto & lr = in.lr[in.lr.size() - 1 - idx];
        static_vector<term_expr*, 8> added_uses{};
        for (i32_t i = 0; i < in.size(); ++i) {
          if (lr.fields[i] < 0) {
            continue;
          }
          auto & f = in[i][lr.fields[i]];
          if (f.moved_to) {
            continue;
          }
          if (!f.consume_inserted) {
            if (!f.consumed_at || !f.consumed_at_used || f.uses.size() != 1 || f.uses.back().te != f.consumed_at) {
              for (auto & use : f.uses) {
                if (use.te == f.consumed_at) {
                  continue;
                }
                bool did_find = false;
                for (term_expr * u : added_uses) {
                  if (u == use.te) {
                    did_find = true;
                  }
                }
                if (did_find) {
                  continue;
                }
                added_uses.push_back(use.te);
                red.statements.emplace_back(use.te, use.te, use.by_ref);
              }
            }
            if (!f.consumed_at) {
              f.consumed_at = make_release_expr(idx, f, loc);
              f.consumed_at_used = false;
            }
            red.statements.emplace_back(f.consumed_at, f.consumed_at_used ? f.consumed_at : nullptr, false);
            f.consume_inserted = true;
          }
          f.uses.clear();
        }
      }

      J_A(RNN,NODISC) term_expr* make_release_expr(i32_t stack_index, input_stack_val & val, const source_location & loc) {
        term_expr * te = ::new term_expr(reduction_arg(stack_index, arg_type::capture_reduction_field, val.field->type, val.field->offset, val.nt));
        te->loc = loc;
        term_exprs args;
        if (val.field->release_extra_arg) {
          term_expr * extra_te = copy(val.field->release_extra_arg, loc);
          extra_te->resolve(scope, g.mod->normal_resolver);
          args.push_back(extra_te);
        }
        te = ::new term_expr(te_member_fn_call{te, val.field->type->release, static_cast<term_exprs &&>(args)});
        te->loc = loc;
        te->compute_hash(scope);
        return te;
      }
    };
  }

  void add_reductions(grammar & g, expr_scope & scope, rule * J_NOT_NULL r) {
    reduction_resolver{g, scope, r}.visit(r->match->root, pat_type_state(g.mod));
  }
}
