#pragma once

#include "meta/rules/pattern_tree.hpp"
#include "meta/rules/args.hpp"
#include "meta/errors.hpp"
#include "meta/rules/term_expr.hpp"
#include "meta/rules/common.hpp"

namespace j::meta::inline rules {
  struct nt_data_field;

  /// Action to perform on a reduction.
  enum class reduction_type : u8_t {
    /// Reduce nonterminal.
    reduce,
    /// Copy a reduction of the same type.
    reduce_copy,
    /// Replace current node with something else.
    rewrite,
    /// Deletes a reduced token, without changing state.
    erase,
    /// Erase all folowing children from the node.
    truncate,
    /// Update statement, e.g. `!!SideFx += $1`.
    stmt,
    ///
    change_type,
    /// Pop reduction depth items.
    pop,
  };

  struct reduction_statement final {
    J_BOILERPLATE(reduction_statement, CTOR_NE, EQ)

    J_A(AI,ND,HIDDEN) inline reduction_statement(term_expr * J_NOT_NULL release_expr, term_expr * consuming_te, bool by_ref) noexcept
      : release_expr(release_expr),
        consuming_te(consuming_te),
        by_ref(by_ref)
    { }

    term_expr * release_expr = nullptr;
    term_expr * consuming_te = nullptr;
    bool by_ref = false;

    J_A(AI,ND,HIDDEN) void replace(const term_expr * J_NOT_NULL from, term_expr * J_NOT_NULL to) noexcept {
      J_ASSERT(from != to);
      if (release_expr == from) {
        release_expr = to;
      }
      if (consuming_te == from) {
        consuming_te = to;
      }
    }
  };

  struct reduction_statements final : vector<reduction_statement> {
    void replace(const term_expr * J_NOT_NULL from, term_expr * J_NOT_NULL to) noexcept;
  };

  struct reduction final {
    reduction_type type = reduction_type::reduce;

    i8_t length = 0U;
    i8_t depth_difference = 0U;
    i32_t size = 0;
    reduction_args args{};
    nt_p nt = nullptr;
    nt_data_type * data_type = nullptr;
    source_location loc{};

    reduction_statements statements;

    J_A(AI,ND) inline reduction() noexcept = default;

    reduction(const reduction & rhs) noexcept;
    reduction & operator=(const reduction & rhs) noexcept;

    reduction(reduction && rhs) noexcept;
    reduction & operator=(reduction && rhs) noexcept;

    J_A(AI,ND) inline reduction(reduction_type type, source_location loc) noexcept
      : type(type),
        loc(loc)
    { }

    J_A(AI,ND) inline reduction(reduction_type type, nt_p nt, source_location loc) noexcept
      : type(type),
        nt(nt),
        loc(loc)
    { }

    J_A(ND) inline explicit reduction(reduction_type type, nt_p nt = nullptr, i8_t depth = 0, source_location loc = {}) noexcept
      : type(type),
        length(depth),
        nt(nt),
        loc(loc)
    { }

    reduction(reduction_type type, reduction_args && args, nt_p nt = nullptr, i8_t depth = 0, source_location loc = {}) noexcept;

    [[nodiscard]] pair<i32_t> extent() const noexcept;

    [[nodiscard]] bool operator==(const reduction & rhs) const noexcept;

    void resolve(const pat_tree & tree, const pat_branch & branch, expr_scope & scope) noexcept;
  };

  struct reduction_group final {
    vector<reduction> reds;
    nt_p target_nt = nullptr;
    term_expr * predicate = nullptr;
    i32_t precedence = 0;
    u32_t hash = 0;
    strings::string name;
    source_location loc;

    J_A(AI,ND) inline reduction_group() noexcept = default;

    reduction_group(nt_p J_NOT_NULL target_nt, source_location loc) noexcept;

    reduction & push_back(reduction && red) noexcept;

    [[nodiscard]] i32_t depth() const noexcept;
    [[nodiscard]] pair<i32_t> extent() const noexcept;

    J_A(RNN) reduction * insert_before(reduction * J_NOT_NULL at, reduction && red) noexcept;
    J_A(RNN) reduction * insert_after(reduction * J_NOT_NULL at, reduction && red) noexcept;

    [[nodiscard]] bool operator==(const reduction_group & rhs) const noexcept;

    void resolve(const pat_tree & tree, const pat_branch & branch, expr_scope & scope) noexcept;
    void compute_hash() noexcept;

    J_A(AI,NODISC) inline u32_t size() const noexcept { return reds.size(); }
    J_A(AI,NODISC) inline reduction & front() noexcept { return reds.front(); }
    J_A(AI,NODISC) inline reduction * begin() noexcept { return reds.begin(); }
    J_A(AI,NODISC) inline reduction * end() noexcept { return reds.end(); }
    J_A(AI,NODISC) inline const reduction * begin() const noexcept { return reds.begin(); }
    J_A(AI,NODISC) inline const reduction * end() const noexcept { return reds.end(); }
    J_A(AI,NODISC) inline reduction & operator[](i32_t i) noexcept { return reds[i]; }
    J_A(AI,NODISC) inline const reduction & operator[](i32_t i) const noexcept { return reds[i]; }
  };

  struct reductions final {
    vector<reduction_group> groups;
    u32_t hash = 0;

    reductions() noexcept;

    reduction_group & push_back(reduction_group && group) noexcept;

    [[nodiscard]] bool has_default() const noexcept;
    [[nodiscard]] bool has_nondefault() const noexcept;

    [[nodiscard]] pair<i32_t> extent() const noexcept;
    [[nodiscard]] arg_counts get_arg_counts() const noexcept;
    [[nodiscard]] reductions operator+(const reductions & rhs) const noexcept;
    [[nodiscard]] bool operator==(const reductions & rhs) const noexcept;
    J_A(AI,ND,NODISC) inline u32_t size() const noexcept { return groups.size(); }
    J_A(AI,ND,NODISC) inline explicit operator bool() const noexcept { return (bool)groups; }
    J_A(AI,ND,NODISC) inline bool operator!() const noexcept { return !groups; }
    J_A(AI,ND,NODISC) inline reduction_group * begin() noexcept { return groups.begin(); }
    J_A(AI,ND,NODISC) inline reduction_group * end() noexcept { return groups.end(); }
    J_A(AI,ND,NODISC) inline const reduction_group * begin() const noexcept { return groups.begin(); }
    J_A(AI,ND,NODISC) inline const reduction_group * end() const noexcept { return groups.end(); }
    J_A(AI,ND,NODISC) inline reduction_group & operator[](i32_t i) noexcept { return groups[i]; }
    J_A(AI,ND,NODISC) inline const reduction_group & operator[](i32_t i) const noexcept { return groups[i]; }

    [[nodiscard]] bool contains_zero_length_reductions() const noexcept;
    [[nodiscard]] bool is_subset_of(const reductions & rhs) const noexcept;
    void resolve(const pat_tree & tree, const pat_branch & branch, expr_scope & scope) noexcept;
    void compute_hash() noexcept;
  };

  struct reduction_hash final  {
    J_A(NODISC) inline u32_t operator()(const mem::shared_ptr<reduction> & r) const noexcept {
      return operator()(*r);
    }
    J_A(NI,ND,NODISC,FLATTEN) inline bool operator()(const mem::shared_ptr<reduction> & lhs, const mem::shared_ptr<reduction> & rhs) const noexcept {
      return *lhs == *rhs;
    }
    J_A(NI,ND,NODISC,FLATTEN) inline bool operator()(const mem::shared_ptr<reduction> & lhs, const reduction & rhs) const noexcept {
      return *lhs == rhs;
    }
    J_A(NI,NODISC,FLATTEN) u32_t operator()(const reduction & r) const noexcept;
  };

  struct reductions_hash final  {
    J_A(AI,ND,NODISC) inline bool operator()(const red_p & lhs, const red_p & rhs) const noexcept {
      return lhs == rhs || *lhs == *rhs;
    }
    J_A(AI,ND,NODISC) inline bool operator()(const red_p & lhs, const reductions & rhs) const noexcept {
      return *lhs == rhs;
    }
    J_A(AI,NODISC,ND) inline u32_t operator()(const red_p & r) const noexcept {
      return r->hash;
    }
    J_A(AI,NODISC,ND) inline u32_t operator()(const reductions & r) const noexcept {
      return r.hash;
    }
  };

  struct reductions_const_key final  {
    using type = reductions;
    using arg_type = const reductions &;
    using getter_type = reductions;
    J_A(AI,NODISC,ND) inline const reductions & operator()(const red_p & r) const noexcept {
      return *r;
    }
  };
}
