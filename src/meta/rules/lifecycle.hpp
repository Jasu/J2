#pragma once

#include "meta/rules/nonterminal.hpp"
#include "containers/static_vector.hpp"

namespace j::meta::inline rules {
  struct pat_tree;

  struct input_stack_val;

  struct stack_ref final {
    J_BOILERPLATE(stack_ref, EQ)
    i32_t stack_offset = -1, field_index = -1;
    J_A(AI,NODISC,ND,HIDDEN) inline explicit operator bool() const noexcept { return stack_offset >= 0 && field_index >= 0; }
    J_A(AI,NODISC,ND,HIDDEN) inline bool operator!() const noexcept { return stack_offset < 0 || field_index < 0; }
  };

  struct output_stack_val final {
    term_expr * te = nullptr;
    te_type type{};
    stack_ref reference{};

    J_A(AI,NODISC,HIDDEN) inline static output_stack_val ref(i32_t stack_offset, i32_t field_index, te_type && type, term_expr * te = nullptr) noexcept {
      J_ASSERT(stack_offset >= 0 && field_index >= 0);
      return {te, (te_type &&)type, stack_ref{stack_offset, field_index}};
    }

    J_A(AI,NODISC,HIDDEN) inline static output_stack_val alloc(term_expr * J_NOT_NULL te, te_type && type) noexcept {
      return {te, (te_type &&)type};
    }

    J_A(AI,NODISC,HIDDEN) inline output_stack_val wrap(term_expr * te) const noexcept {
      return {te, type, reference};
    }

    J_A(AI,NODISC,HIDDEN) inline output_stack_val wrap(term_expr * te, te_type && type) const noexcept {
      return {te, (te_type &&)type, reference};
    }

    J_A(AI,NODISC,ND,HIDDEN) inline explicit operator bool() const noexcept { return te || reference; }
    J_A(AI,NODISC,ND,HIDDEN) inline bool operator!() const noexcept { return !te && !reference; }
    J_A(AI,NODISC,ND,HIDDEN) inline bool empty() const noexcept { return !te && !reference; }

    J_A(AI,NODISC,ND,HIDDEN) inline bool is_ref() const noexcept { return (bool)reference; }
    J_A(AI,NODISC,ND,HIDDEN) inline bool is_alloc() const noexcept { return te; }
  };

  struct input_use final {
    term_expr * te = nullptr;
    bool by_ref = false;
    J_BOILERPLATE(input_use, CTOR_NE)
    J_A(AI,ND,HIDDEN) inline input_use(term_expr * J_NOT_NULL te, bool by_ref) noexcept
      : te(te), by_ref(by_ref)
    { }
  };

  struct input_stack_val final {
    term_expr * consumed_at = nullptr;
    bool consumed_at_used = true;
    bool consume_inserted = false;
    stack_ref moved_to{};
    nt_p nt = nullptr;
    const nt_data_field * field = nullptr;
    static_vector<input_use, 8> uses{};

    J_A(AI,ND,HIDDEN) inline input_stack_val() noexcept = default;
    J_A(AI,ND,HIDDEN) inline input_stack_val(nt_p J_NOT_NULL nt, const nt_data_field * J_NOT_NULL f) noexcept
      : nt(nt),
        field(f)
    { }

    J_A(AI,ND,HIDDEN) inline input_stack_val(nt_p J_NOT_NULL nt, const nt_data_field * J_NOT_NULL f, term_expr * consumed_at, stack_ref moved_to) noexcept
      : consumed_at(consumed_at),
        moved_to(moved_to),
        nt(nt),
        field(f)
    { }

    J_A(AI,NODISC,ND,HIDDEN) inline explicit operator bool() const noexcept { return nt; }
    J_A(AI,NODISC,ND,HIDDEN) inline bool operator!() const noexcept { return !nt; }

    J_A(AI,NODISC,ND,HIDDEN) inline bool is_dead() const noexcept {
      return nt && (consumed_at || moved_to);
    }

    J_A(AI,NODISC,ND,HIDDEN) inline bool is_alive() const noexcept {
      return nt && !consumed_at;
    }

    J_A(AI,ND,HIDDEN) inline void use(term_expr * J_NOT_NULL te, bool by_ref) noexcept {
      for (auto & u : uses) {
        if (u.te == te) {
          u.by_ref |= by_ref;
          return;
        }
      }
      uses.emplace_back(te, by_ref);
    }

    J_A(AI,ND,HIDDEN) inline void consume(term_expr * J_NOT_NULL te, bool use_result) noexcept {
      J_ASSERT(nt && !consumed_at && !moved_to);
      consumed_at = te;
      consumed_at_used = use_result;
    }

    J_A(AI,ND,HIDDEN) inline void move(stack_ref to) noexcept {
      J_ASSERT(nt && !consumed_at && !moved_to && to);
      moved_to = to;
    }

    J_A(AI,ND,HIDDEN) inline void move(i32_t i, i32_t j) noexcept {
      J_ASSERT(nt && !consumed_at && !moved_to && i >= 0 && j >= 0);
      moved_to = {i, j};
    }

    J_A(AI,ND,HIDDEN) inline void enter_branch() noexcept {
      uses.clear();
    }

    void exit_if(term_expr * J_NOT_NULL te, const input_stack_val & branch0, const input_stack_val & branch1);
  };

  struct lr_stack_val final {
    nt_p nt = nullptr;
    terminal_set term{};
    i32_t fields[4] = {-1, -1, -1, -1};
    J_A(AI,ND,HIDDEN) inline lr_stack_val() noexcept = default;
    J_A(AI,ND,HIDDEN) inline explicit lr_stack_val(nt_p nt) noexcept
      : nt(nt)
    { }

    J_A(AI,ND,HIDDEN) inline explicit lr_stack_val(const terminal_set & term) noexcept
      : term(term)
    { J_ASSERT(term); }
  };

  using input_stack_state = static_vector<input_stack_val, 8>;
  using output_stack_state = static_vector<output_stack_val, 8>;
  using lr_state = static_vector<lr_stack_val, 8>;

  struct input_state final : static_vector<input_stack_state, 4> {
    // J_A(AI,ND,HIDDEN) inline input_state() noexcept = default;
    lr_state lr;
    explicit input_state(const module * J_NOT_NULL mod) noexcept;
    using static_vector::operator[];
    J_A(AI,HIDDEN,NODISC) inline input_stack_val & operator[](stack_ref ref) noexcept {
      return operator[](ref.field_index)[ref.stack_offset];
    }
    J_A(AI,HIDDEN,NODISC) inline const input_stack_val & operator[](stack_ref ref) const noexcept {
      return operator[](ref.field_index)[ref.stack_offset];
    }
    void clear(const module * J_NOT_NULL mod) noexcept;
  };

  struct output_state final : static_vector<output_stack_state, 4> {
    lr_state lr;
    explicit output_state(const module * J_NOT_NULL mod) noexcept;
    using static_vector::operator[];
    J_A(AI,HIDDEN,NODISC) inline output_stack_val & operator[](stack_ref ref) noexcept {
      return operator[](ref.field_index)[ref.stack_offset];
    }
    J_A(AI,HIDDEN,NODISC) inline const output_stack_val & operator[](stack_ref ref) const noexcept {
      return operator[](ref.field_index)[ref.stack_offset];
    }
    void clear(const module * J_NOT_NULL mod) noexcept;
  };

  struct pat_type_state final {
    explicit pat_type_state(module * J_NOT_NULL m) noexcept;

    input_state input;
    output_state output;
    J_A(AI,NODISC,HIDDEN) inline input_state snapshot() noexcept { return input; }
    void enter_branch() noexcept;
    input_state exit_branch(const input_state & snapshot) noexcept;
    void exit_if(term_expr * J_NOT_NULL te, const input_state & snap, const input_state & b0, const input_state & b1);

    void push_back(const pat_p & pat) noexcept;
  };

  void add_reductions(grammar & g, expr_scope & scope, rule * J_NOT_NULL r);
}
