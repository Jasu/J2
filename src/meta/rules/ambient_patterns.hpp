#pragma once

#include "meta/term.hpp"
#include "meta/rules/pat_item.hpp"

namespace j::meta {
  struct term;
}

namespace j::meta::inline rules {
  struct pat_state final {
    term * parent = nullptr;
    bool has_multiple_parents = false;
    u32_t min_operands_left = 0U;
    u32_t max_operands_left = U32_MAX;

    void enter(term * J_NOT_NULL t) noexcept {
      parent = t;
      min_operands_left = max_operands_left = t->num_inputs;
      if (t->has_dynamic_input) {
        --min_operands_left;
        max_operands_left = U32_MAX;
      }
    }

    [[nodiscard]] pat_state next(const terminal_set & lookahead) noexcept {
      bool has_non_exit = lookahead.size() - lookahead.has(exit);
      return {
        parent,
        has_multiple_parents,
        min_operands_left ? min_operands_left - 1 : 0,
        has_non_exit ? (max_operands_left == U32_MAX ? U32_MAX : max_operands_left - 1) : 0,
      };
    }

    inline pat_state & operator+=(const pat_state & rhs) noexcept {
      has_multiple_parents |= rhs.has_multiple_parents;
      has_multiple_parents |= parent && rhs.parent &&  parent != rhs.parent;
      parent = has_multiple_parents ? nullptr : parent ? parent : rhs.parent;
      min_operands_left = j::min(min_operands_left, rhs.min_operands_left);
      max_operands_left = j::max(max_operands_left, rhs.max_operands_left);
      return *this;
    }
  };

  struct ambient_pat_key final {
    terminal parent{255};
    terminal entry{255};
    J_A(AI,NODISC) inline bool operator==(const ambient_pat_key &) const noexcept = default;
  };

  struct ambient_pat_key_hash final {
    J_A(AI,NODISC,ND,HIDDEN) inline u32_t operator()(ambient_pat_key key) const noexcept {
      return crc32(1 + key.parent.index, key.entry.index);
    }
  };

  struct ambient_patterns final {
    J_A(AI) inline ambient_patterns(grammar * J_NOT_NULL g) noexcept
      : g(*g)
    { }
    hash_map<ambient_pat_key, pat_item_vec, ambient_pat_key_hash> pats;
    pat_item_vec recovery_generic;
    pat_item_vec recovery_one;
    pat_item_vec recovery_zero;
    grammar & g;

    void initialize() noexcept;

    void add_for_parents(const terminal_set & terminals, const rule & rule, const pat_p & pat) noexcept;
    void add_for_enter(const terminal_set & parents, const rule & rule, const pat_p & pat) noexcept;
    void add_for_enter_next(const terminal_set & parents, terminal t, const rule & rule, const pat_p & pat) noexcept;

    [[nodiscard]] span<const pat_item> get_for_state(const pat_state & s) const noexcept;
    [[nodiscard]] span<const pat_item> get_for_entry(terminal t, const pat_state & s) const noexcept;
    [[nodiscard]] span<const pat_item> recovery_for_state(const pat_state & s, const terminal_set & begin) const noexcept;
  };
}

J_DECLARE_EXTERN_HASH_MAP(j::meta::ambient_pat_key, j::meta::pat_item_vec, HASH(j::meta::ambient_pat_key_hash));
