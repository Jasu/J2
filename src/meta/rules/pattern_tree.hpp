#pragma once

#include "meta/rules/args.hpp"
#include "hzd/iterators.hpp"
#include "meta/rules/common.hpp"
#include "meta/common.hpp"
#include "containers/vector.hpp"
#include "mem/shared_ptr_fwd.hpp"
#include "meta/rules/terminals.hpp"
#include "meta/rules/args.hpp"

namespace j::meta::inline rules {
  struct nonterminal;
  struct grammar;
  struct reductions;

  enum class pat_type : u8_t {
    terminal,
    nonterminal,
    alternation,
    reduce,
  };

  enum pat_nonterminal_t : u8_t { pat_nonterminal_v };
  enum pat_alternation_t : u8_t { pat_alternation_v };
  enum pat_terminal_t : u8_t { pat_terminal_v };
  enum pat_reduce_t : u8_t { pat_reduce_v };

  using pat_vec = vector<pat_p>;

  struct pat_elem final {
    nt_p nt = nullptr;
    terminal t{0};
    J_A(AI,ND) inline pat_elem() noexcept = default;
    J_A(AI) inline explicit pat_elem(nt_p J_NOT_NULL nt) noexcept
      : nt(nt)
    { }
    J_A(AI) inline explicit pat_elem(terminal t) noexcept
      : t(t)
    { }
    J_A(AI,NODISC) inline bool operator==(const pat_elem &) const noexcept = default;
    J_A(AI,NODISC) inline bool operator==(nt_p J_NOT_NULL p) const noexcept {
      return nt == p;
    }
    J_A(AI,NODISC) inline bool operator==(terminal v) const noexcept {
      return !nt && t == v;
    }
    [[nodiscard]] bool operator<(const pat_elem & rhs) const noexcept;
  };

  using pat_elem_vec = vector<pat_elem>;

  struct pat_elem_vec_hash final  {
    [[nodiscard]] u32_t operator()(const pat_elem_vec & v) const noexcept;
    [[nodiscard]] bool operator()(const pat_elem_vec & lhs, const pat_elem_vec & rhs) const noexcept;
  };

  [[nodiscard]] bool overlaps(const pat_elem_vec & lhs, const pat_elem_vec & rhs) noexcept;

  struct pat_paths final  {
    J_A(AI,ND) inline pat_paths() noexcept = default;
    explicit pat_paths(hash_set<pat_elem_vec, pat_elem_vec_hash, pat_elem_vec_hash> && vecs) noexcept;

    hash_set<pat_elem_vec, pat_elem_vec_hash, pat_elem_vec_hash> vecs{};
    [[nodiscard]] static pat_paths next(const pat_paths & prev, pat_elem e) noexcept;
    J_A(AI,ND,NODISC,HIDDEN) static inline pat_paths next(const pat_paths & prev, terminal t) noexcept {
      return next(prev, pat_elem(t));
    }
    J_A(AI,ND,NODISC,HIDDEN) static inline pat_paths next(const pat_paths & prev, nt_p J_NOT_NULL nt) noexcept {
      return next(prev, pat_elem(nt));
    }
    bool add(pat_elem_vec && rhs) noexcept;
    bool add(const pat_elem_vec & rhs) noexcept;
    bool add(const pat_paths & rhs) noexcept;
    [[nodiscard]] vector<pat_elem_vec> items() const noexcept;
    J_A(AI,NODISC,HIDDEN) inline explicit operator bool() const noexcept { return (bool)vecs; }
    J_A(AI,NODISC,HIDDEN) inline bool operator!() const noexcept { return !(bool)vecs; }
  };



  struct pat final {
    J_A(ND) inline pat() noexcept = default;

    J_A(ND) inline pat(pat_nonterminal_t, nt_p J_NOT_NULL nt, pat_precedence precedence = {}) noexcept
      : type(pat_type::nonterminal),
        precedence(precedence),
        nonterm(nt)
    { }

    pat(pat_alternation_t, pat_p && alt, pat_precedence precedence = {}) noexcept;
    pat(pat_alternation_t, pat_vec && alts, pat_precedence precedence = {}) noexcept;

    J_A(ND) inline pat(pat_reduce_t, red_p && red, pat_precedence precedence = {}) noexcept
      : type(pat_type::reduce),
        precedence(precedence),
        reduce(static_cast<red_p &&>(red)),
        first(epsilon)
    { }

    template<typename T>
    J_A(ND) inline pat(pat_terminal_t, T && terminals, pat_precedence precedence = {}) noexcept
      : type(pat_type::terminal),
        precedence(precedence),
        first(static_cast<T &&>(terminals))
    {
      J_ASSERT(first);
    }

    pat_type type = pat_type::nonterminal;
    pat_precedence precedence{};
    u32_t hash = 0U;
    pat_p next{};
    pat_vec alternates{};
    nt_p nonterm{};
    red_p reduce{};
    terminal_set first{};
    pat_paths paths{};
    pat_p temp{};

    [[nodiscard]] bool operator==(const pat & rhs) const noexcept;

    [[nodiscard]] pair<i32_t> length_cur_level(i32_t level = 0) const noexcept;
    [[nodiscard]] pair<i32_t> extent() const noexcept;
  };


  struct pat_branch final {
    pat_p * tail = nullptr;
    i32_t depth = 0;
    i32_t reduction_depth = 0;
    i8_t num_root_elements = 0;
    i8_t depth_difference = 0;
    pat_captures caps{};
    nt_p reduce_as = nullptr;

    void set_reduction(red_p && red, pat_precedence prec) noexcept;
  private:
    friend struct pat_builder;
    pat & previous() const noexcept;
    void set_paths(const pat_p & p) noexcept;
    void set_capture(i8_t index, pat_capture caps, bool overwrite = false) noexcept;
  };

  struct pat_tree final {
    pat_p root;
    noncopyable_vector<pat_branch> branches;
    bool is_single_ctor;
    bool is_single_nonterminal;
    bool has_lookbehind;
    bool has_depth_difference;
  };

  J_A(RNN,NODISC) pat_tree * make_empty_pat_tree() noexcept;

  struct pat_term;
  using pat_term_p = j::mem::shared_ptr<pat_term>;
  using pat_terms_t = vector<pat_term_p>;

  struct pat_term final {
    enum pat_term_type : u8_t {
      type_none,

      type_any,
      type_term,
      type_ctor,
      type_exit,

      type_nt,

      type_anchor,
      type_end,
    };
    J_A(AI,HIDDEN,ND) inline pat_term() noexcept = default;

    J_A(AI,HIDDEN,ND) explicit inline pat_term(pat_term_type type, i32_t capture = -1) noexcept
      : type(type),
        capture_index(capture)
    { J_ASSERT(type != type_none); }

    J_A(AI,HIDDEN,ND) inline pat_term(pat_term_type type, term * J_NOT_NULL t) noexcept
    : type(type),
      name{.term = t}
    { }

    J_A(AI,HIDDEN,ND) explicit inline pat_term(nt_p J_NOT_NULL nt, i32_t capture = -1) noexcept
    : type(type_nt),
      capture_index(capture),
      name{.nt = nt}
    { }

    void clear() noexcept;

    J_A(AI,ND,HIDDEN) inline void capture(i32_t idx) noexcept {
      J_ASSERT(idx >= 0 && capture_index == -1);
      capture_index = idx;
    }

    J_A(AI,HIDDEN,ND) inline ~pat_term() { clear(); }

    pat_term_type type = type_none;
    i8_t capture_index = -1;
    i8_t nt_captures[6] = {0};
    pat_terms_t next;
    union {
      nt_p nt = nullptr;
      term * term;
    } name = {};

    [[nodiscard]] pat_term_p clone() const noexcept;
  };

  struct pat_builder_ctx;

  struct pat_builder final {
    const module & mod;
    pat_layer layer;
    i8_t precedence;
    i32_t index;
    noncopyable_vector<pat_branch> branches;
    pat_p root;
    nt_p any;
    nt_p anys;

    pat_builder(const module & mod, pat_layer layer, i8_t precedence, i32_t index) noexcept;

    void visit(const pat_terms_t & pat);
    void visit(const pat_term & pat);
    J_A(RNN,NODISC) pat_tree * build() noexcept;
  private:
    void visit(pat_builder_ctx ctx, const pat_terms_t & pat, pat_p * J_NOT_NULL to);
    void visit(pat_builder_ctx ctx, const pat_term & pat, pat_p * J_NOT_NULL to);
    J_A(HIDDEN,NODISC) pat_precedence prec(pat_prec_type t) const noexcept;
  };

  struct pat_builder_ctx final {
    i32_t depth = 0;
    i32_t reduction_depth = 0;
    i32_t reduction_tree_depth = 0;
    /// \note Don't convert this to use a single var, otherwise num_root_elements would get incremented.
    i32_t reduction_tree_depth_zero = 0;
    i32_t num_root_elements = 0;
    i32_t lookahead_depth = 0;
    pat_captures caps{};

    J_A(AI,ND,HIDDEN) void add_element(i32_t depth) {
      this->depth += depth;
      if (!reduction_tree_depth) {
        ++num_root_elements;
      }
      reduction_depth += depth;
    }

    J_A(AI,ND,HIDDEN) void add_depth(i32_t depth) {
      this->depth += depth;
      reduction_depth += depth;
    }
  };

  pat_p add_alternate(pat_p && to, pat_p alt) noexcept;
  pat_p filter_alternate(const grammar & g, pat_p && alt) noexcept;
  pat_p copy(const pat_p & from) noexcept;
  pat_p copy(const pat_p & from, pat_precedence prec) noexcept;
  u32_t compute_hash(const pat_p & r) noexcept;

  struct pat_hash final  {
    J_A(NI,ND,NODISC,FLATTEN) inline u32_t operator()(const mem::shared_ptr<pat> & r) const noexcept {
      return operator()(*r);
    }
    J_A(NI,ND,NODISC,FLATTEN) inline bool operator()(const mem::shared_ptr<pat> & lhs, const mem::shared_ptr<pat> & rhs) const noexcept {
      return *lhs == *rhs;
    }
    J_A(NI,ND,NODISC,FLATTEN) inline bool operator()(const mem::shared_ptr<pat> & lhs, const pat & rhs) const noexcept {
      return *lhs == rhs;
    }
    J_A(AI,ND,NODISC) u32_t operator()(const pat & r) const noexcept {
      return r.hash;
    }
  };
}

J_DECLARE_EXTERN_HASH_SET(j::meta::pat_elem_vec, HASH(j::meta::pat_elem_vec_hash), KEYEQ(j::meta::pat_elem_vec_hash));
