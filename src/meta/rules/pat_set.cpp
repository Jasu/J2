#include "pat_set.hpp"
#include "strings/format.hpp"
#include "meta/module.hpp"
#include "meta/rules/dump.hpp"
#include "logging/global.hpp"
#include "containers/hash_map.hpp"
#include "meta/rules/grammar.hpp"

J_DEFINE_EXTERN_HASH_SET(j::meta::pat_set_pp, HASH(j::meta::pat_set_hash), KEYEQ(j::meta::pat_set_hash), CONST_KEY(j::meta::pat_set_const_key));
J_DEFINE_EXTERN_HASH_MAP(j::meta::terminal, j::meta::action, HASH(j::meta::terminal_hash));
J_DEFINE_EXTERN_HASH_MAP(j::meta::nt_p, j::meta::pat_goto, HASH(j::meta::nonterminal_hash));

namespace j::meta::inline rules {
  namespace s = strings;

  namespace {
    void dump_item(const grammar & g, const pat_item & item) noexcept {
      terminal_set la{item.lookahead};
      s::string la_str;
      if (g.terminals.any_terminal.is_subset_of(la)) {
        la -= g.terminals.any_terminal;
        la_str += "Any";
      }
      for (terminal t : la) {
        if(la_str) {
          la_str.push_back(' ');
        }
        la_str += g.terminals.name_of(g.mod, t);
      }
      J_DEBUG("   {} / {}", dump_str(g.mod, g, item.pat), la_str);

    }

    void dump_item_set(const grammar & g, const pat_item_set & set) noexcept {
      for (auto & p : set.items()) {
        dump_item(g, p);
      }
    }

    [[maybe_unused]] void dump_item_sets(const grammar & g, vector<pat_set_pp> sets) noexcept {
      u32_t i = 0U;
      for (auto s : sets) {
        J_DEBUG("Set #{}", i++);
        dump_item_set(g, s->set);
      }
    }
  }

  pat_set_pp::pat_set_pp(pat_set_p * J_NOT_NULL p) noexcept
    : set(p)
  { }

  action::action(red_p && red) noexcept
      : type(action_type::reduce),
        reduce(static_cast<red_p &&>(red))
    { }

  action::action(const red_p & red) noexcept
      : type(action_type::reduce),
        reduce(red)
    { }

  action::action(pat_set_pp to) noexcept
      : type(action_type::shift),
        to(to)
    { }

  action::action(action_type type, const red_p & red) noexcept
      : type(type),
        reduce(red)
    { }

  void action::reset() noexcept {
    type = action_type::none;
    reduce.reset();
    to.reset();
  }

  pat_goto::pat_goto(pat_set_p * J_NOT_NULL p, bool is_recovery) noexcept
    : pat_set_pp(p),
      is_recovery(is_recovery)
  { }

  pat_goto::pat_goto(pat_set_pp p, bool is_recovery) noexcept
    : pat_set_pp(p),
      is_recovery(is_recovery)
  { }


  bool pat_set::combine(grammar & g, const pat_set & rhs) noexcept {
    J_ASSERT(*this == rhs);
    bool did_change = false;
    for (auto & p : rhs.set) {
      auto pair = set.emplace(p);
      did_change |= pair.second;
      if (!pair.second && pair.first->pat != p.pat) {
        did_change |= (*pair.first)->paths.add(p->paths);
      }
    }
    state += rhs.state;
    if (did_change) {
      closure(g);
      dirty = true;
    }
    return did_change;
  }

  bool pat_set::is_mergeable_with(const pat_set & rhs, bool debug) noexcept {
    if (unambiguous_action != rhs.unambiguous_action) {
      return false;
    }
    if (unambiguous_non_exit_action != rhs.unambiguous_non_exit_action) {
      return false;
    }

    if (!unambiguous_action) {
      for (auto & a : actions) {
        auto * a2 = rhs.actions.maybe_at(a.first);
        if (!a2 || a.second == *a2) {
          continue;
        }
        if (a.second.type != a2->type) {
          if (debug) {
            J_DEBUG("Fail: Action {} type differs", a.first.index);
          }
          return false;
        }
        if (a.second.type == action_type::shift) {
          if ((a.second.to == this || a.second.to == &rhs)
              && (a2->to == this || a2->to == &rhs))
          {
            continue;
          }
        }
        // else if (a.second.is_reduce() && (a.second.reduce->is_subset_of(*a2->reduce)
        //                                   || a2->reduce->is_subset_of(*a.second.reduce)))
        // {
        //   continue;
        // }
        if (debug) {
          J_DEBUG("Fail: {} {}", (i8_t)a.second.type, a.first.index);
        }
        return false;
      }
    }

    for (auto & g : gotos) {
      auto * g2 = rhs.gotos.maybe_at(g.first);
      if (!g2 || g.second == *g2) {
        continue;
      }
      if ((g.second == this || g.second == &rhs)
          && (*g2 == this || *g2 == &rhs))
      {
        continue;
      }
      if (debug) {
        J_DEBUG("Fail: Goto {}", g.first->name);
      }
      return false;
    }
    return true;
  }

  void pat_set::merge_with(pat_set & rhs) noexcept {
    for (auto & a : rhs.actions) {
      actions.emplace(a.first, a.second);
    }
    for (auto & g : rhs.gotos) {
      gotos.emplace(g.first, g.second);
    }
    for (auto & p : rhs.set) {
      if (p.is_kernel()) {
        auto pair = set.emplace(p);
        if (!pair.second && p.pat != pair.first->pat) {
          pair.first->pat->paths.add(p->paths);
        }
      }
    }
    state += rhs.state;
  }

  pat_set pat_set::next(grammar & g, const nt_p & nt) {
    J_ASSERT(nt);
    pat_set result;
    for (auto & p : set.items()) {
      if (p.starts_with(nt)) {
        p.add_next(result.set);
      }
    }
    if (result) {
      bool is_first = true;
      for (auto & p : result.set) {
        if (p->type != pat_type::reduce) {
          if (state.parent || state.has_multiple_parents) {
            auto s = state.next(p.first());
            if (is_first) {
              result.state = s;
              is_first = false;
            } else {
              result.state += s;
            }
          }
        }
      }
      result.closure(g);
    }
    return result;
  }

  pat_set pat_set::next(grammar & g, terminal t) {
    pat_set result;
    if (state.min_operands_left > 0 && t == exit) {
      return result;
    } else if (state.max_operands_left == 0 && t != exit) {
      return result;
    }
    for (auto & p : set.items()) {
      if (p.starts_with(t)) {
        p.add_next(result.set);
      }
    }
      if (t != exit) {
        result.state.enter(g.mod->terms()[t.index]);
        for (auto & item : g.ambient_pats.get_for_entry(t, state)) {
          add_pat_item(result.set, item);
        }
      }
    if (result) {
      result.closure(g);
    }
    return result;
  }

  void pat_set::compute_hash() noexcept {
    hash = 1U;
    for (auto & item : set) {
      if (item.is_kernel()) {
        hash ^= item.pat->hash;
      }
    }
  }

  bool pat_set::closure(grammar & g) {
    bool did_change = false;

    for (auto & pat : set.items()) {
      did_change |= pat.add_closure(set, state);
    }

    for (auto & item : g.ambient_pats.get_for_state(state)) {
      if (set.emplace(item).second) {
        item.add_closure(set, state);
        did_change = true;
      }
    }

    for (auto & item : g.ambient_pats.recovery_for_state(state, set.terminals)) {
      if (set.emplace(item).second) {
        item.add_closure(set, state);
        did_change = true;
      }
    }

    compute_hash();
    return did_change;
  }

  [[nodiscard]] bool pat_set::operator==(const pat_set & rhs) const noexcept {
    // if (ambient != rhs.ambient) {
    //   return false;
    // }
    if (hash != rhs.hash) {
      return false;
    }
    if (&rhs == this) {
      return true;
    }
    u32_t num_kernel = 0U;
    for (auto & p : set) {
      if (p.is_kernel()) {
        ++num_kernel;
        if (!rhs.set.contains(p)) {
          return false;
        }
      }
    }
    for (auto & p : rhs.set) {
      if (p.is_kernel()) {
        --num_kernel;
      }
    }
    return !num_kernel;
  }

  pat_sets::pat_sets(grammar & g) noexcept
    : any_pat(mem::make_shared<pat>(pat_nonterminal_v, g.any, pat_precedence{pat_layer::standard, 0, pat_prec_type::shift_any}))
  {
    J_ASSERT(g.any && g.anys && g.any != g.anys);
    root = pat_set_pp(::new pat_set_p(mem::make_shared<pat_set>()));
    compute_hash(any_pat);
    add_pat_item(root->set, {any_pat, g.terminals.any_terminal_or_exit});
    root->closure(g);
    sets.emplace_back(root);

    pat_set_set hash;
    hash.emplace(root);
    expand(g, hash);
  }

  pat_set_pp pat_sets::insert(grammar & g, pat_set && set, pat_set_set & hash, bool & did_change) noexcept {
    if (!set) {
      return {};
    }
    if (pat_set_pp * prev = hash.maybe_at(set)) {
      J_ASSERT(*prev && **prev == set);
      did_change |= (*prev)->combine(g, set);
      return *prev;
    }
    did_change = true;
    pat_set_pp ps(::new pat_set_p(mem::make_shared<pat_set>(static_cast<pat_set &&>(set))));
    sets.emplace_back(ps);
    hash.emplace(ps);
    return ps;
  }

  void pat_sets::expand(grammar & g, pat_set_set & hash) noexcept {
    J_ASSERT(sets.size());
    bool did_change;
    do {
      did_change = false;
      for (u32_t index = 0U; index < sets.size(); ++index) {
        pat_set_pp cur = sets[index];
        if (!cur->dirty) {
          continue;
        }
        cur->dirty = false;
        terminal_set visited_terminals{};
        for (auto & p : cur->set.items()) {
          J_ASSERT(p);
          switch (p->type) {
          case pat_type::terminal:
            for (terminal t : p->first) {
              if (visited_terminals.has(t)) {
                continue;
              }
              visited_terminals += t;
              insert(g, cur->next(g, t), hash, did_change);
            }
            break;
          case pat_type::nonterminal:
            insert(g, cur->next(g, p->nonterm), hash, did_change);
            break;
          case pat_type::alternation:
            J_FAIL("Unexpected alternation");
          case pat_type::reduce: break;
          }
        }
      }
    } while (did_change);

    for (auto & set : sets) {
      for (auto & p : set->set.items()) {
        J_ASSERT(p);
        bool is_recovery = p.precedence.layer() == pat_layer::recovery;
        switch (p->type) {
        case pat_type::terminal:
          for (terminal t : p->first) {
            if (pat_set_pp n = insert(g, set->next(g, t), hash, did_change)) {
              auto pair = set->actions.emplace(t, n);
              if (pair.second) {
                pair.first->second.is_recovery = is_recovery;
              }
            }
          }
          break;
        case pat_type::nonterminal:
          if (pat_set_pp n = insert(g, set->next(g, p->nonterm), hash, did_change)) {
            set->gotos.emplace(p->nonterm, n, is_recovery);
          }
          break;
        case pat_type::alternation:
          J_FAIL("Unexpected alternation");
        case pat_type::reduce: {
          for (terminal t : p.lookahead) {
            auto pair = set->actions.emplace(t, p->reduce);
            if (pair.second) {
              pair.first->second.is_recovery = is_recovery;
            } else {
              red_p & cur_red = pair.first->second.reduce;
              if (pair.first->second.type == action_type::reduce
                  && cur_red != p->reduce
                  // && !cur_red->has_default()
                  && p->reduce->has_nondefault()) {
                // J_DEBUG("Reduce-Reduce:\nA:\n{}\nB:\n{}", dump_str(g.mod, pair.first->second.reduce),
                //         dump_str(g.mod, p->reduce));
                pair.first->second.reduce = g.add_reduction_raw(*pair.first->second.reduce + *p->reduce);
              }
            }
          }
          break;
        }
        }
      }
      set->compute_is_unambiguous();
    }

    for (auto & set : sets) {
      bool did_change = false;
      for (auto & p : set->actions) {
        if (p.second.type != action_type::shift) {
          J_ASSERT(p.second.type == action_type::reduce);
          continue;
        }
        pat_set_pp to = p.second.to;
        J_ASSERT(to);
        if (to->unambiguous_action.type != action_type::reduce || to->unambiguous_action.reduce->contains_zero_length_reductions()) {
          continue;
        }
        p.second = action(action_type::shift_reduce, to->unambiguous_action.reduce);
        did_change = true;
      }
      if (did_change) {
        set->compute_is_unambiguous();
      }
    }

    // for (auto & set : sets) {
    //   if (set->unambiguous_non_exit_action) {
    //     for (auto & a : set->actions) {
    //       if (a.first != exit) {
    //         set->unambiguous_non_exit_action = set->actions.begin()->second;
    //         break;
    //       }
    //     }
    //   }
    //   if (set->unambiguous_action) {
    //     set->unambiguous_action = set->actions.begin()->second;
    //   }
    // }

    root->index = 1U;
    for (auto & set : sets) {
      for (auto & p : set->actions) {
        if (p.second.to) {
          p.second.to->index = 1U;
        }
      }
      for (auto & p : set->gotos) {
        p.second->index = 1U;
      }
    }

    for (pat_set_pp * it = sets.begin(); it != sets.end();) {
      if ((*it)->index) {
        ++it;
      } else {
        it = sets.erase(it);
      }
    }


    vector<pat_set_pp> new_sets(sets);
  again:
    for (auto it = new_sets.begin() + 1, end = new_sets.end(); it != end; ++it) {
      for (auto it2 = it + 1; it2 != end; ++it2) {
        if ((*it)->is_mergeable_with(**it2)) {
          (*it)->merge_with(**it2);
          auto old = *(it2->set);
          for (auto & s : sets) {
            if (*s.set == old) {
              *s.set = *it->set;
            }
          }
          new_sets.erase(it2);
          goto again;
        }
      }
    }
    sets = new_sets;


    u32_t index = 0U;
    for (auto & set : sets) {
      set->index = index++;
    }

    // u32_t idx1 = 30, idx2 = 31;
    // pat_set_pp ss[]{sets[idx1], sets[idx2]};
    // for (auto & s : ss) {
    //   J_DEBUG("Set {}, U:{} U(!E):{}", s->index, (i8_t)s->unambiguous_action.type, (i8_t)s->unambiguous_non_exit_action.type);
    // }
    // J_DEBUG("  Merge? {}/{}", ss[0]->is_mergeable_with(*ss[1]), ss[1]->is_mergeable_with(*ss[0]));
    // J_DEBUG("  UEq? {}", ss[0]->unambiguous_action == ss[1]->unambiguous_action);
    // J_DEBUG("  UEq(!E)? {}", ss[0]->unambiguous_non_exit_action == ss[1]->unambiguous_non_exit_action);
    // J_DEBUG("  R0 {}", dump_str(nullptr, ss[0]->actions.at(exit).reduce));
    // J_DEBUG("  R1 {}", dump_str(nullptr, ss[1]->actions.at(exit).reduce));
    // ss[0]->is_mergeable_with(*ss[1], true);
  }

  void pat_set::compute_is_unambiguous() noexcept {
    unambiguous_non_exit_action.reset();
    if (actions.empty()) {
      unambiguous_action.reset();
      return;
    }
    unambiguous_action = actions.begin()->second;

    for (auto & a2 : actions) {
      if (unambiguous_action != a2.second) {
        unambiguous_action.reset();
      }
      if (a2.first == exit) {
        continue;
      }
      if (!unambiguous_non_exit_action) {
        unambiguous_non_exit_action = a2.second;
      } else if (unambiguous_non_exit_action != a2.second) {
        unambiguous_non_exit_action.reset();
        break;
      }
    }
  }
  const pat_paths pat_set::paths() const noexcept {
    pat_paths result = paths(false, false);
    if (!result) {
      result = paths(false, true);
    }
    if (!result) {
      result = paths(true, false);
    }
    if (!result) {
      result = paths(true, true);
    }
    return result;
  }

  const pat_paths pat_set::paths(bool include_recovery, bool include_reduce) const noexcept {
    pat_paths paths;
    for (auto & p : set) {
      if (p.is_kernel() && (include_recovery || p->precedence.layer() != pat_layer::recovery) && (include_reduce || p->type != pat_type::reduce)) {
        paths.add(p->paths);
      }
    }
    return paths;
  }
}
