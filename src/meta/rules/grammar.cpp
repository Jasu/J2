#include "grammar.hpp"
#include "meta/rules/rule.hpp"
#include "meta/rules/lifecycle.hpp"
#include "logging/global.hpp"
#include "meta/expr.hpp"
#include "lisp/air/exprs/expr.hpp"
#include "meta/rules/dump.hpp"
#include "meta/rules/reduction.hpp"
#include "meta/module.hpp"
#include "meta/rules/nonterminal.hpp"
#include "meta/term.hpp"
#include "containers/hash_map.hpp"

J_DEFINE_EXTERN_HASH_SET(j::mem::shared_ptr<j::meta::rules::reductions>, HASH(j::meta::rules::reductions_hash), KEYEQ(j::meta::rules::reductions_hash), CONST_KEY(j::meta::reductions_const_key));

namespace j::meta::inline rules {
  namespace s = strings;
  namespace {
    bool compute_first(pat & p);

    J_A(AI,NODISC,ND) static inline bool is_ambient(const rule * J_NOT_NULL r) noexcept {
      // if (r->debug) {
      //   J_DEBUG("Debug", !!r->has_parent, r->match->has_lookbehind, r->match->is_single_ctor, r->match->is_single_nonterminal, r->match->depth_difference);
      // }
      return r->has_parent || r->match->has_lookbehind || r->match->has_depth_difference;
    }
  }

  grammar::grammar(module * J_NOT_NULL mod) noexcept
    : mod(mod),
      ambient_pats(this)
  {
  }

  [[nodiscard]] grammar * make_grammar(module & mod) {
    grammar * const result = ::new grammar{&mod};
    result->any = result->get_nt("Any");
    result->anys = result->get_nt("Anys");

    {
      pat_p anys = mem::make_shared<pat>(pat_nonterminal_v, result->any, pat_precedence{pat_layer::standard, -3, pat_prec_type::shift_any});
      anys->next = mem::make_shared<pat>(pat_nonterminal_v, result->anys, pat_precedence{pat_layer::standard, -3, pat_prec_type::shift_anys});
      anys->next->paths = pat_paths::next(anys->paths, result->any);
      anys->next->next = mem::make_shared<pat>(pat_reduce_v, result->add_reduction_raw(reduction{reduction_type::reduce, {}, result->anys, 2}), pat_precedence{pat_layer::standard, -3, pat_prec_type::reduce_anys});
      anys->next->next->paths = pat_paths::next(anys->next->paths, result->anys);
      result->anys->add_pat(static_cast<pat_p &&>(anys));

      anys = mem::make_shared<pat>(pat_reduce_v, result->add_reduction_raw(reduction{reduction_type::reduce, {}, result->anys, 0}), pat_precedence{pat_layer::standard, -3, pat_prec_type::reduce_anys});
      result->anys->add_pat(static_cast<pat_p &&>(anys));
    }

    for (term * t : mod.terms()) {
      J_ASSERT(t && result->terminals.size == t->index);
      result->terminals.add(t->name, t->num_inputs - t->has_dynamic_input, t->has_dynamic_input ? 255 : t->num_inputs);

      pat_builder builder{mod, pat_layer::standard, -2, t->index};
      pat_term pt(pat_term::type_term, t);
      builder.visit(pt);
      pat_tree * tree = builder.build();
      J_ASSERT(tree->branches.size() == 1);
      pat_branch & b = tree->branches.front();
      J_ASSERT(b.depth == 2 + t->num_inputs);
      b.set_reduction(result->add_reduction_raw(reduction(reduction_type::reduce, {}, result->any, -1)), {pat_layer::standard, -2, pat_prec_type::reduce_any, t->index});
      result->any->add_pat(static_cast<pat_p &&>(tree->root));
      ::delete tree;
    }


    i32_t index = 0U;
    expr_scope_root root_scope = mod.make_root_scope();
    for (rule * r : mod.rules) {
      J_SCOPE_INFO(source_loc(r->loc));
      J_ASSERT_NOT_NULL(r, r->match, r->match->branches);
      if (is_ambient(r)) {
        continue;
      }
      J_ASSERT_NOT_NULL(r->reds);

      add_reductions(*result, root_scope, r);
      for (auto & branch : r->match->branches) {
        J_ASSERT(branch.tail && *branch.tail && (*branch.tail)->type == pat_type::reduce);
        for (auto & g : *(*branch.tail)->reduce) {
          if (!g.target_nt) {
            // !None reduction
            continue;
          }
          // TODO FIRST sets of different branches not included.
          g.target_nt->add_pat(copy(r->match->root));
        }
      }
      ++index;
    }

    for (rule * r : mod.rules) {
      J_SCOPE_INFO(source_loc(r->loc));
      if (!is_ambient(r)) {
        continue;
      }

      i32_t index = 0;
      for (auto & branch : r->match->branches) {
        reductions reds = r->reds;
        for (auto & r : reds) {
          for (auto & r : r) {
            r.nt = nullptr;
          }
        }
        reds.resolve(*r->match, branch, root_scope);
        branch.set_reduction(result->add_reduction_raw(static_cast<reductions &&>(reds)), {pat_layer::ambient, (i8_t)r->precedence, pat_prec_type::rewrite, index});
      }
      pat_p pattern = copy(r->match->root);

      auto parent = r->has_parent ? terminal_set{r->parent_terms.bitmask} : terminal_set{};
      compute_first(*pattern);
      if (r->debug) {
        debug_dump_pat(*result, pattern);
      }
      if (pattern->type != pat_type::nonterminal || r->match->has_lookbehind) {
        result->ambient_pats.add_for_enter(parent, *r, static_cast<pat_p &&>(pattern));
      } else {
        result->ambient_pats.add_for_parents(parent, *r, static_cast<pat_p &&>(pattern));
      }
      ++index;
    }

    result->ambient_pats.initialize();

    compute_first(*result);
    for (auto nt : mod.nonterminals()) {
      J_ASSERT_NOT_NULL(nt);
      if (nt->pat) {
        compute_hash(nt->pat);
      }
    }
    for (auto & pv : result->ambient_pats.pats) {
      for (auto & p : pv.second) {
        compute_hash(p.pat);
      }
    }
    return result;
  }

  namespace {
    bool compute_first(pat & p) {
      bool result = false;
      if (p.next) {
        result = compute_first(*p.next);
      }

      terminal_set first;
      switch (p.type) {
      case pat_type::reduce: case pat_type::terminal:
        return result;
      case pat_type::nonterminal: {
        J_ASSERT(p.nonterm && p.nonterm->pat, "No pat in {}", p.nonterm->name);
        first = p.nonterm->pat->first;
        break;
      }
      case pat_type::alternation: {
        for (auto & alt : p.alternates) {
          if (!alt) {
            first += epsilon;
          } else {
            result |= compute_first(*alt);
            first += alt->first;
          }
        }
        break;
      }
      }
      if (first.has(epsilon) && p.next) {
        first -= epsilon;
        first += p.next->first;
      }
      if (!first.is_subset_of(p.first)) {
        p.first += first;
        result = true;
      }
      return result;
    }
  }

  void compute_first(grammar & g) {
    bool did_add;
    do {
      did_add = false;
      for (auto nt : g.mod->nonterminals()) {
        if (nt->pat) {
          did_add |= compute_first(*nt->pat);
        }
      }
      for (auto amb : g.ambient_pats.pats) {
        for (auto & p : amb.second) {
          did_add |= compute_first(*p.pat);
        }
      }
    } while (did_add);
  }

  [[nodiscard]] red_p grammar::add_reduction_raw(struct reductions && r) {
    J_ASSERT(r && r == r);
    r.compute_hash();
    if (red_p * p = reductions.maybe_at(r)) {
      J_ASSERT_NOT_NULL(*p);
      return *p;
    } else {
      red_p p2 = mem::make_shared<struct reductions>(static_cast<struct reductions &&>(r));
      reductions.emplace(p2);
      J_ASSERT(p2 && *p2 && *p2 == r);
      return p2;
    }
  }

  [[nodiscard]] red_p grammar::add_reduction_raw(reduction && r) {
    reduction_group g;
    g.push_back(static_cast<reduction &&>(r));
    struct reductions rs;
    rs.push_back(static_cast<reduction_group &&>(g));
    return add_reduction_raw(static_cast<struct reductions &&>(rs));
  }

  [[nodiscard]] pat_p grammar::add_reduction(struct reductions && r, pat_precedence precedence) {
    J_ASSERT(r);
    return mem::make_shared<pat>(pat_reduce_v, add_reduction_raw(static_cast<struct reductions &&>(r)), precedence);
  }

  [[nodiscard]] pat_p grammar::add_reduction(reduction && r, pat_precedence precedence) {
    return mem::make_shared<pat>(pat_reduce_v, add_reduction_raw(static_cast<reduction &&>(r)), precedence);
  }

  J_A(NODISC,RNN) nt_p grammar::get_nt(strings::const_string_view name) const noexcept {
    return static_cast<nt_p>(mod->node_at(name, node_nonterminal));
  }
}
