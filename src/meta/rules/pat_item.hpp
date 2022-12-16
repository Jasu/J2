#pragma once

#include "containers/hash_map_fwd.hpp"
#include "meta/rules/pattern_tree.hpp"
#include "meta/rules/nonterminal.hpp"

namespace j::meta::inline rules {
  struct pat_state;

  struct pat_item_lr0_hash;
  struct pat_item;
  using pat_item_vec = vector<pat_item>;
  struct pat_item_set;

  struct pat_item final {
    pat_p pat{};
    terminal_set lookahead{};
    u32_t index = 0;
    pat_precedence precedence{};

    J_A(AI,ND,HIDDEN) inline pat_item() noexcept = default;

    pat_item(const pat_p & pat, const terminal_set & lookahead, u32_t index = 0) noexcept;

    pat_item(pat_p && pat, const terminal_set & lookahead, u32_t index = 0U) noexcept;

    J_A(AI,NODISC,ND,RNN) inline struct pat * operator->() noexcept { return pat.get(); }
    J_A(AI,NODISC,ND,RNN) inline const struct pat * operator->() const noexcept { return pat.get(); }
    J_A(AI,NODISC,ND) inline bool operator!() const noexcept { return !pat; }
    J_A(AI,NODISC,ND) inline operator bool() const noexcept { return (bool)pat; }

    J_A(AI,NODISC,ND,HIDDEN) inline bool is_kernel() const noexcept {
      return index || (!index && pat && pat->type == pat_type::nonterminal && !pat->next);
    }

    J_A(AI,NODISC,ND,HIDDEN) inline bool lr0_eq(const pat_item & rhs) const noexcept {
      return pat ? *pat == *rhs.pat : !rhs.pat;
    }

    inline pat_item & operator+=(const pat_item & rhs) noexcept {
      J_ASSERT(lr0_eq(rhs));
      lookahead += rhs.lookahead;
      return *this;
    }

    J_A(NODISC) inline bool starts_with(terminal t) const noexcept {
      return pat && pat->type == pat_type::terminal && pat->first.has(t);
    }

    J_A(NODISC,AI) inline bool starts_with(const nt_p & t) const noexcept {
      return pat && pat->nonterm == t;
    }

    [[nodiscard]] terminal_set first() const noexcept {
      J_ASSERT(pat);
      terminal_set result = pat->first;
      if (result.has_epsilon()) {
        result -= epsilon;
        result += lookahead;
      }
      return result;
    }

    bool add_next(pat_item_set & h) const noexcept;
    bool add_closure(pat_item_set & h, const pat_state & s) const noexcept;
  };

  struct pat_item_lr0_hash final {
    J_A(AI,NODISC,ND) inline bool operator()(const pat_item & lhs, const pat_item & rhs) const noexcept {
      return lhs.pat == rhs.pat || (lhs.pat && *lhs.pat == *rhs.pat);
    }
    J_A(AI,NODISC,ND) inline bool operator()(const pat_item & lhs, const pat_p & rhs) const noexcept {
      return lhs.pat == rhs || (rhs && *lhs.pat == *rhs);
    }
    J_A(AI,NODISC,ND) inline bool operator()(const pat_item & lhs, const pat * rhs) const noexcept {
      return lhs.pat.get() == rhs || (rhs && *lhs.pat == *rhs);
    }
    J_A(AI,NODISC,ND) inline u32_t operator()(const pat * rhs) const noexcept {
      return rhs ? rhs->hash : 0U;
    }
    J_A(AI,NODISC,ND) inline u32_t operator()(const pat_p & rhs) const noexcept {
      return rhs ? rhs->hash : 0U;
    }
    J_A(AI,NODISC,ND) inline u32_t operator()(const pat_item & rhs) const noexcept {
      return rhs.pat ? rhs.pat->hash : 0U;
    }
  };

  struct pat_item_const_key final {
    using type = const pat*;
    using arg_type = const pat *;
    using getter_type = pat_item;
    J_A(AI,NODISC,ND,RNN) inline const pat* operator()(const pat_p & rhs) const noexcept {
      return rhs.get();
    }
    J_A(AI,NODISC,ND,RNN) inline const pat* operator()(const pat_item & rhs) const noexcept {
      return rhs.pat.get();
    }
  };

  struct pat_item_set final {
    pat_item_set() noexcept;

    hash_set<pat_item, pat_item_lr0_hash, pat_item_lr0_hash, pat_item_const_key> set;
    terminal_set terminals;

    J_A(AI,ND,NODISC) inline auto begin() noexcept { return set.begin(); }
    J_A(AI,ND,NODISC) inline auto begin() const noexcept { return set.begin(); }
    J_A(AI,ND,NODISC) inline auto end() noexcept { return set.end(); }
    J_A(AI,ND,NODISC) inline auto end() const noexcept { return set.end(); }
    J_A(AI,ND,NODISC) inline u32_t size() const noexcept { return set.size(); }
    J_A(AI,ND,NODISC) inline explicit operator bool() const noexcept { return (bool)set; }
    J_A(AI,ND,NODISC) inline bool operator!() const noexcept { return !(bool)set; }

    J_A(AI,ND,NODISC) inline pat_item * maybe_at(const pat_item & item) noexcept {
      return set.maybe_at(item);
    }

    J_A(AI,ND,NODISC) inline bool contains(const pat_item & item) const noexcept {
      return set.contains(item);
    }

    template<typename T>
    J_A(ND) inline auto add(T && item) noexcept {
      auto p = set.insert(static_cast<T &&>(item));
      if (!p.second) {
        if (!item.lookahead.is_subset_of(p.first->lookahead)) {
          p.first->lookahead += item.lookahead;
          p.second = true;
        }
        if (p.first->precedence > item.precedence) {
          p.first->precedence = item.precedence;
          p.second = true;
        }
        if (p.first->index < item.index) {
          p.first->index = item.index;
          p.second = true;
        }
      } else {
        terminals += item->first;
      }
      return p;
    }

    template<typename... Ts>
    J_A(AI,ND) inline auto emplace(Ts && ... ts) noexcept {
      return add(pat_item(static_cast<Ts &&>(ts)...));
    }

    [[nodiscard]] pat_item_vec items() const noexcept;
  };

  bool add_pat_item(pat_item_set & h, const pat_item & item) noexcept;
}

J_DECLARE_EXTERN_HASH_SET(j::meta::pat_item, HASH(j::meta::pat_item_lr0_hash), KEYEQ(j::meta::pat_item_lr0_hash), CONST_KEY(j::meta::pat_item_const_key));
