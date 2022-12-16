#include "ambient_patterns.hpp"
#include "meta/module.hpp"
#include "meta/rules/rule.hpp"
#include "algo/quicksort.hpp"
#include "containers/hash_map.hpp"
#include "meta/term.hpp"
#include "meta/rules/grammar.hpp"

J_DEFINE_EXTERN_HASH_MAP(j::meta::ambient_pat_key, j::meta::pat_item_vec, HASH(j::meta::ambient_pat_key_hash));

namespace j::meta::inline rules {
  namespace {
    J_A(AI) inline i32_t pat_item_cmp(const pat_item & item) noexcept {
      return item.precedence.precedence;
    }

    J_A(AI,NODISC) inline ambient_pat_key for_parent(u8_t index) noexcept {
      return {terminal{index}, terminal{255}};
    }

    J_A(AI,NODISC) inline ambient_pat_key for_parent(terminal parent) noexcept {
      return {parent, terminal{255}};
    }

    J_A(AI,NODISC) inline ambient_pat_key for_entry(terminal entry) noexcept {
      return {terminal{255}, entry};
    }
  }
  void ambient_patterns::initialize() noexcept {
    pat_precedence prec{pat_layer::recovery, 0, pat_prec_type::shift_anys};
    pat_p recovery_zero_pat = mem::make_shared<pat>(pat_terminal_v, exit, pat_precedence{pat_layer::recovery, 0, pat_prec_type::shift});
    recovery_zero_pat->next = g.add_reduction(reduction(reduction_type::reduce, {}, g.any, -1), pat_precedence{pat_layer::recovery, 0, pat_prec_type::reduce_any});
    pat_p recovery_generic_pat = mem::make_shared<pat>(pat_nonterminal_v, g.anys, pat_precedence{pat_layer::recovery, 0, pat_prec_type::shift_anys});
    recovery_generic_pat->next = recovery_zero_pat;
    recovery_zero_pat->paths.add(pat_paths::next(recovery_generic_pat->paths, g.anys));

    pat_p recovery_one_pat = mem::make_shared<pat>(pat_nonterminal_v, g.any, pat_precedence{pat_layer::recovery, 0, pat_prec_type::shift_any});
    recovery_one_pat->next = recovery_zero_pat;
    recovery_zero_pat->paths.add(pat_paths::next(recovery_one_pat->paths, g.any));
    recovery_zero_pat->next->paths = pat_paths::next(recovery_zero_pat->paths, exit);

    compute_hash(recovery_generic_pat);
    compute_hash(recovery_one_pat);
    recovery_zero.emplace_back(recovery_zero_pat, g.terminals.any_terminal_or_exit, 0);
    recovery_one.emplace_back(recovery_one_pat, g.terminals.any_terminal_or_exit, 0);
    recovery_generic.emplace_back(recovery_generic_pat, g.terminals.any_terminal_or_exit, 0);

    for (auto & p : pats) {
      if (p.first.parent.index != 255 && p.first.entry.index != 255) {
        if (const pat_item_vec * pat = pats.maybe_at(for_parent(p.first.parent))) {
          for (auto & p2 : *pat) {
            p.second.push_back(p2);
          }
        }
        if (const pat_item_vec * pat = pats.maybe_at(for_entry(p.first.entry))) {
          for (auto & p2 : *pat) {
            p.second.push_back(p2);
          }
        }
        algo::quicksort(p.second.begin(), p.second.end(), pat_item_cmp);
      }
    }
  }

  void ambient_patterns::add_for_parents(const terminal_set & terminals, const rule & rule, const pat_p & pat) noexcept {
    J_ASSERT_NOT_NULL(terminals, pat);
    J_ASSERT(!terminals.has_exit());
    pat_item item{pat, rule.not_at_exit ? g.terminals.any_terminal : g.terminals.any_terminal_or_exit, 1 };
    for (terminal t : terminals) {
      pats[for_parent(t)].push_back(item);
    }
  }

  void ambient_patterns::add_for_enter(const terminal_set & parents, const rule & rule, const pat_p & pat) noexcept {
    J_ASSERT_NOT_NULL(pat);
    if (pat->type == pat_type::alternation) {
      for (auto & p : pat->alternates) {
        add_for_enter(parents, rule, p);
      }
      return;
    }
    J_ASSERT(pat->type == pat_type::terminal && pat->first.size() == 1 && pat->next);
    add_for_enter_next(parents, pat->first.front(), rule, pat->next);
  }

  void ambient_patterns::add_for_enter_next(const terminal_set & parents, terminal t, const rule & rule, const pat_p & pat) noexcept {
    J_ASSERT_NOT_NULL(pat);
    J_ASSERT(!t.is_special() && !parents.has_exit());
    if (pat->type == pat_type::alternation) {
      for (auto & p : pat->alternates) {
        add_for_enter_next(parents, t, rule, p);
      }
      return;
    }
    compute_hash(pat);
    const bool not_at_exit = rule.not_at_exit
      || pat->extent().second < 0 || g.terminals.def_of(t).min_operands > pat->length_cur_level().second;
    pat_item item{pat, not_at_exit ? g.terminals.any_terminal : g.terminals.any_terminal_or_exit, 1};
    if (!parents) {
      pats[for_entry(t)].push_back(item);
    } else {
      for (terminal parent : parents) {
        pats[ambient_pat_key{parent, t}].push_back(item);
      }
    }
  }

  [[nodiscard]] span<const pat_item> ambient_patterns::get_for_state(const pat_state & s) const noexcept {
    if (s.parent && s.max_operands_left) {
      if (const pat_item_vec * p = pats.maybe_at(for_parent(s.parent->index))) {
        return *p;
      }
    }
    return {};
  }

  [[nodiscard]] span<const pat_item> ambient_patterns::get_for_entry(terminal t, const pat_state & s) const noexcept {
    if (t != exit) {
      if (s.parent) {
        if (const pat_item_vec * p = pats.maybe_at(ambient_pat_key{{s.parent->index}, t})) {
          return *p;
        }
      }
      if (const pat_item_vec * p = pats.maybe_at(for_entry(t))) {
        return *p;
      }
    }
    return {};
  }

  [[nodiscard]] span<const pat_item> ambient_patterns::recovery_for_state(const pat_state & s, const terminal_set & begin) const noexcept {
    if (s.max_operands_left == 0 || (!s.parent && !s.has_multiple_parents)) {
      return {};
    }

    const bool matches_exit = begin.has(exit);
    if (g.terminals.any_terminal.is_subset_of(begin)) {
      if (s.min_operands_left || matches_exit) {
        return {};
      }
      return recovery_zero;
    }
    if (s.max_operands_left == 1 && (s.min_operands_left == 1 || matches_exit)) {
      return recovery_one;
    }
    return recovery_generic;
  }
}
