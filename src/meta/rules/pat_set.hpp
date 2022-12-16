#pragma once

#include "meta/rules/pattern_tree.hpp"
#include "meta/rules/reduction.hpp"
#include "meta/rules/nonterminal.hpp"
#include "meta/rules/ambient_patterns.hpp"
#include "meta/rules/pat_item.hpp"

namespace j::meta::inline rules {
  struct pat_set;
  using pat_set_p = mem::shared_ptr<pat_set>;
  struct pat_set_pp {
    J_BOILERPLATE(pat_set_pp, CTOR_NE_ND)
    pat_set_p * set = nullptr;

    explicit pat_set_pp(pat_set_p * J_NOT_NULL p) noexcept;

    J_A(AI,NODISC,ND,RNN) inline pat_set * operator->() const noexcept {
      return set->get();
    }
    J_A(AI,NODISC,ND) inline pat_set & operator*() const noexcept {
      return *set->get();
    }
    [[nodiscard]] inline bool operator!() const noexcept {
      return !set || !*set;
    }
    [[nodiscard]] inline explicit operator bool() const noexcept {
      return set && *set;
    }
    J_A(AI,NODISC,ND) inline bool operator==(const pat_set_pp & rhs) const noexcept {
      return set ? rhs.set && *set == *rhs.set : !rhs.set;
    }
    J_A(AI,NODISC,ND) inline bool operator==(const pat_set * rhs) const noexcept {
      return set ? set->get() == rhs : !rhs;
    }

    J_A(AI,HIDDEN,ND) inline void reset() noexcept { set = nullptr; }
  };


  enum class action_type : u8_t {
    none,
    shift,
    reduce,
    shift_reduce,
  };

  struct action final {
    J_BOILERPLATE(action, CTOR_NE_ND)

    action_type type = action_type::none;
    red_p reduce{};
    pat_set_pp to{};
    bool is_recovery = false;



    J_A(AI,NODISC) inline explicit operator bool() const noexcept { return type != action_type::none; }
    J_A(AI,NODISC) inline bool operator!() const noexcept { return type == action_type::none; }

    J_A(AI,NODISC) inline bool operator==(const action & rhs) const noexcept {
      if (type != rhs.type) {
        return false;
      }
      switch (type) {
      case action_type::none:
        return true;
      case action_type::shift:
        return to == rhs.to;
      case action_type::reduce:
      case action_type::shift_reduce:
        return *reduce == *rhs.reduce;
      }
    }

    J_A(AI,NODISC) inline bool is_reduce() const noexcept { return type >= action_type::reduce; }

    action(red_p && red) noexcept;

    action(const red_p & red) noexcept;

    action(pat_set_pp to) noexcept;

    action(action_type type, const red_p & red) noexcept;

    void reset() noexcept;
  };

  struct pat_goto final : pat_set_pp {
    J_A(AI,ND) inline pat_goto() noexcept = default;
    explicit pat_goto(pat_set_p * J_NOT_NULL p, bool is_recovery = false) noexcept;
    explicit pat_goto(pat_set_pp p, bool is_recovery = false) noexcept;

    bool is_recovery = false;
  };

  struct pat_set final {
    J_BOILERPLATE(pat_set, CTOR_NE_ND, MOVE_NE_ND)

    pat_item_set set;
    hash_map<terminal, action, terminal_hash> actions;
    hash_map<nt_p, pat_goto, nonterminal_hash> gotos;
    u32_t index = 0;
    u32_t hash = 0;
    bool dirty = true;
    action unambiguous_action{};
    action unambiguous_non_exit_action{};
    pat_state state{};

    void compute_is_unambiguous() noexcept;
    [[nodiscard]] const pat_paths paths() const noexcept;
    [[nodiscard]] const pat_paths paths(bool include_recovery, bool include_reduce) const noexcept;

    [[nodiscard]] pat_set next(grammar & g, const nt_p & nt);
    [[nodiscard]] pat_set next(grammar & g, terminal t);
    bool combine(grammar & g, const pat_set & rhs) noexcept;
    [[nodiscard]] bool is_mergeable_with(const pat_set & rhs, bool debug = false) noexcept;
    void merge_with(pat_set & rhs) noexcept;

    bool closure(grammar & g);
    void compute_hash() noexcept;
    void compute_hash_no_pats() noexcept;

    J_A(AI,NODISC,HIDDEN) inline u32_t size() const noexcept { return set.size(); }
    J_A(AI,NODISC,HIDDEN) inline bool operator!() const noexcept { return !set; }
    J_A(AI,NODISC,HIDDEN) inline explicit operator bool() const noexcept { return (bool)set; }

    [[nodiscard]] bool operator==(const pat_set & rhs) const noexcept;
  };


  struct pat_set_hash final  {
    J_A(AI,ND,NODISC,HIDDEN) inline bool operator()(const pat_set_pp & lhs, const pat_set & rhs) const noexcept {
      return *lhs == rhs;
    }
    J_A(AI,ND,NODISC,HIDDEN) inline bool operator()(const pat_set_pp & lhs, const pat_set_pp & rhs) const noexcept {
      return *lhs == *rhs;
    }
    J_A(AI,ND,NODISC,HIDDEN) inline u32_t operator()(const pat_set & s) const noexcept {
      return s.hash;
    }
    J_A(AI,ND,NODISC,HIDDEN) inline u32_t operator()(const pat_set_pp & s) const noexcept {
      return s->hash;
    }
  };

struct pat_set_const_key final  {
  using type = pat_set;
  using arg_type = const pat_set &;
  using getter_type = pat_set;
  J_A(AI,ND,NODISC) inline const pat_set & operator()(const pat_set_pp & s) const noexcept {
    return *s;
  }
};

using pat_set_set = hash_set<pat_set_pp, pat_set_hash, pat_set_hash, pat_set_const_key>;
  struct pat_sets final {
    vector<pat_set_pp> sets;
    pat_sets(grammar & g) noexcept;
    pat_set_pp root;

    J_A(AI,NODISC,HIDDEN) inline u32_t size() const noexcept { return sets.size(); }
    J_A(AI,NODISC,HIDDEN) inline bool operator!() const noexcept { return !sets; }
    J_A(AI,NODISC,HIDDEN) inline explicit operator bool() const noexcept { return (bool)sets; }

    J_A(AI,NODISC,HIDDEN) inline pat_set_pp & operator[](u32_t i) noexcept { return sets[i]; }
    J_A(AI,NODISC,HIDDEN) inline const pat_set_pp & operator[](u32_t i) const noexcept { return sets[i]; }

    J_A(AI,NODISC,HIDDEN) inline pat_set_pp * begin() noexcept { return sets.begin(); }
    J_A(AI,NODISC,HIDDEN) inline pat_set_pp * end() noexcept { return sets.end(); }
    J_A(AI,NODISC,HIDDEN) inline const pat_set_pp * begin() const noexcept { return sets.begin(); }
    J_A(AI,NODISC,HIDDEN) inline const pat_set_pp * end() const noexcept { return sets.end(); }


  private:
    pat_set_pp insert(grammar & g, pat_set && set, pat_set_set & hash, bool & did_change) noexcept;
    void expand(grammar & g, pat_set_set & hash) noexcept;
    pat_p any_pat;
  };
}

J_DECLARE_EXTERN_HASH_SET(j::meta::pat_set_pp, HASH(j::meta::pat_set_hash), KEYEQ(j::meta::pat_set_hash), CONST_KEY(j::meta::pat_set_const_key));
J_DECLARE_EXTERN_HASH_MAP(j::meta::terminal, j::meta::action, HASH(j::meta::terminal_hash));
J_DECLARE_EXTERN_HASH_MAP(j::meta::nt_p, j::meta::pat_goto, HASH(j::meta::nonterminal_hash));
