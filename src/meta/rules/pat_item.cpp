#include "pat_item.hpp"
#include "meta/rules/ambient_patterns.hpp"
#include "algo/quicksort.hpp"
#include "containers/hash_map.hpp"

J_DEFINE_EXTERN_HASH_SET(j::meta::pat_item, HASH(j::meta::pat_item_lr0_hash), KEYEQ(j::meta::pat_item_lr0_hash), CONST_KEY(j::meta::pat_item_const_key));

namespace j::meta::inline rules {
  namespace {
    J_A(AI) inline i32_t pat_item_cmp(const pat_item & item) noexcept {
      return item.precedence.precedence;
    }
  }

  pat_item::pat_item(const pat_p & pat, const terminal_set & lookahead, u32_t index) noexcept
    : pat(pat),
      lookahead(lookahead),
      index(index),
      precedence(pat->precedence)
  {
    J_ASSERT(this->lookahead);
  }

  pat_item::pat_item(pat_p && pat, const terminal_set & lookahead, u32_t index) noexcept
    : pat(static_cast<pat_p &&>(pat)),
      lookahead(lookahead),
      index(index),
      precedence(pat->precedence)
  {
    J_ASSERT(this->lookahead);
  }

  bool add_pat_item(pat_item_set & h, const pat_item & item) noexcept {
    auto pair = h.emplace(item);
    if (!pair.second) {
      if (pair.first->pat == item.pat) {
        return false;
      }
      bool did_add = item.lookahead.is_subset_of(pair.first->lookahead);
      if (did_add) {
        pair.first->lookahead += item.lookahead;
      }
      did_add |= pair.first->pat->paths.add(item->paths);
      return did_add;
    }
    return true;
  }

  bool pat_item::add_next(pat_item_set & h) const noexcept {
    if (!pat || !pat->next) {
      return false;
    }
    pat_p & p = pat->next;
    if (p->type == pat_type::alternation) {
      bool did_add = false;
      for (auto & alt : p->alternates) {
        J_ASSERT(alt && alt->type != pat_type::alternation);
        did_add |= add_pat_item(h, {alt, lookahead, index + 1});
      }
      return did_add;
    }
    return add_pat_item(h, {p, lookahead, index + 1});
  }

  bool pat_item::add_closure(pat_item_set & h, const pat_state & s) const noexcept {
    if (!pat || pat->type != pat_type::nonterminal) {
      return false;
    }
    bool can_have_exit = !s.min_operands_left;
    bool must_have_exit = !s.max_operands_left;
    terminal_set new_lookahead{(pat->next && pat->next->type != pat_type::reduce) ? pat->next->first : lookahead};
    if (new_lookahead.has_epsilon()) {
      new_lookahead -= epsilon;
      new_lookahead += lookahead;
      J_ASSERT(!new_lookahead.has_epsilon());
    }
    J_ASSERT(new_lookahead);
    auto p = pat->nonterm->pat;
    if (p->type == pat_type::alternation) {
      bool did_add = false;
      for (auto & alt : p->alternates) {
        J_ASSERT(alt && alt->type != pat_type::alternation);
        pat_item new_item{alt, new_lookahead};
        new_item.precedence = precedence;
        if (!can_have_exit || must_have_exit) {
          auto f = new_item.first();
          const bool has_exit = f.has(exit);
          if (!can_have_exit && has_exit && f.size() == 1) {
            continue;
          }
          if (must_have_exit && !has_exit) {
            continue;
          }
        }
        if (add_pat_item(h, new_item)) {
          did_add = true;
          new_item.add_closure(h, s);
        }
      }
      return did_add;
    } else {
      pat_item new_item{p, new_lookahead};
      new_item.precedence = precedence;
      if (!can_have_exit || must_have_exit) {
        auto f = new_item.first();
        const bool has_exit = f.has(exit);
        if (!can_have_exit && has_exit && f.size() == 1) {
          return false;
        }
        if (must_have_exit && !has_exit) {
          return false;
        }
      }
      if (add_pat_item(h, new_item)) {
        new_item.add_closure(h, s);
        return true;
      }
    }
    return false;
  }
  pat_item_set::pat_item_set() noexcept {
  }

  [[nodiscard]] vector<pat_item> pat_item_set::items() const noexcept {
    pat_item_vec result(set.size());
    for (auto & item : set) {
      result.emplace_back(item);
    }
    algo::quicksort(result.begin(), result.end(), pat_item_cmp);
    return static_cast<pat_item_vec &&>(result);
  }
}
