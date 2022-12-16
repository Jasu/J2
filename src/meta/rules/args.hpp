#pragma once

#include "containers/vector.hpp"
#include "mem/shared_ptr.hpp"
#include "exceptions/assert_lite.hpp"
#include "meta/rules/common.hpp"
#include "meta/rules/terminals.hpp"

namespace j::meta {
  struct val_type;
  struct expr_scope;
}

namespace j::meta::inline rules {
  struct pat_capture;
  struct term_expr;
  using copy_cb_t = void (*)(const term_expr * J_NOT_NULL, term_expr * J_NOT_NULL, void *) noexcept;

  enum class arg_type : u8_t {
    none,
    /// Capture a single term of the production, as in `E -> vec($1)` or `E -> vec:$1(...)`.
    capture_term,
    /// Capture collect terms on stack
    capture_term_field,
    /// Capture a field of a previous reduction, as in `E -> NewVec($1)`.
    capture_reduction_field,
    /// Capture a term representing "this" in e.g. `LeftRecurse* Foo { x -> ! FooField = 1; }`
    capture_this_reduction_nt,
    /// Copy the reduction structure completely, as in `foo:$1 -> !Copy $1`.
    capture_reduction_copy,
    /// Copy the reduction structure completely, as in `Neq:$1 -> Eq:$1`.
    capture_reduction_copy_nt,
    capture_reduction_copy_sz,
    /// Select a term that has not yet been shifted: `vec(_ // $1)`.
    select_term_ahead,
    /// Holds a reference ($1) in a reduction argumet until resolved against pat_captures.
    reference,

    reduction_copy_min = capture_this_reduction_nt,
    reduction_copy_max = capture_reduction_copy_sz,
  };

  struct reduction_arg_ctor_info;
  struct reduction_arg final {
    arg_type type = arg_type::none;
    i8_t stack_offset = 0U;
    u8_t field_offset = 0U;
    i8_t capture_index = 0U;
    union {
      val_type * value_type = nullptr;
      nt_data_type * data_type;
      nt_p nt;
      u32_t size;
    };
    nt_p debug_nt = nullptr;
    J_A(AI) inline reduction_arg() noexcept : value_type(nullptr) { }

    J_A(AI,ND,HIDDEN) inline explicit reduction_arg(i8_t capture_index) noexcept
      : type(arg_type::reference),
        capture_index(capture_index),
        value_type(nullptr)
    { }

    J_A(AI,ND,HIDDEN) inline reduction_arg(i8_t stack_offset, arg_type type, nt_p nt, u8_t field_offset = 0) noexcept
      : type(type),
        stack_offset(stack_offset),
        field_offset(field_offset),
        nt(nt)
    { }

    J_A(AI,ND,HIDDEN) inline reduction_arg(i8_t stack_offset, arg_type type, nt_data_type * dt, u8_t field_offset = 0) noexcept
      : type(type),
        stack_offset(stack_offset),
        field_offset(field_offset),
        data_type(dt)
    { }

    reduction_arg(i8_t stack_offset, arg_type type, val_type * value_type, u8_t field_offset = 0, nt_p debug_nt = nullptr) noexcept;

    void resolve(const pat_capture & cap);

    J_A(AI,NODISC) inline bool operator!() const noexcept {
      return type == arg_type::none;
    }
    J_A(AI,NODISC) inline explicit operator bool() const noexcept {
      return type != arg_type::none;
    }

    [[nodiscard]] inline bool operator==(const reduction_arg & rhs) const noexcept {
      return type == rhs.type
        && (type == arg_type::reference
            ? capture_index == rhs.capture_index
            : (stack_offset == rhs.stack_offset && field_offset == rhs.field_offset
               && (type == arg_type::capture_reduction_copy_sz ? size == rhs.size : value_type == rhs.value_type)));
    }

    [[nodiscard]] i32_t stack_depth() const noexcept;

    J_A(AI,NODISC) bool is_nonterminal() const noexcept {
      return type >= arg_type::reduction_copy_min && type <= arg_type::reduction_copy_max;
    }
  };

  struct pat_capture final {
    arg_type type = arg_type::none;
    i8_t stack_offset = 0;
    u8_t field_offset = 0;
    val_type * value_type = nullptr;
    nt_p nt = nullptr;
    J_A(AI) inline pat_capture() noexcept = default;

    J_A(AI,ND) inline pat_capture(arg_type type, i8_t stack_offset, nt_p J_NOT_NULL nt) noexcept
      : type(type),
        stack_offset(stack_offset),
        nt(nt)
    {  }

    J_A(AI,ND) inline pat_capture(arg_type type, i8_t stack_offset, u8_t field_offset = 0) noexcept
      : type(type),
        stack_offset(stack_offset),
        field_offset(field_offset)
    { }

    J_A(AI,ND) inline pat_capture(arg_type type, i8_t stack_offset, val_type * value_type, nt_p nt, u8_t field_offset = 0) noexcept
      : type(type),
        stack_offset(stack_offset),
        field_offset(field_offset),
        value_type(value_type),
        nt(nt)
    { }


    J_A(AI,NODISC) inline bool operator!() const noexcept {
      return type == arg_type::none;
    }
    J_A(AI,NODISC) inline explicit operator bool() const noexcept {
      return type != arg_type::none;
    }
    J_A(AI,NODISC) inline bool operator==(const pat_capture &) const noexcept = default;
  };

  enum class rewrite_action : u8_t {
    none,
    replace,
  };

  struct reduction_value final {
    term_expr * expr = nullptr;
    rewrite_action rewrite_action = rewrite_action::none;
    i8_t rewrite_offset = 0;

    J_BOILERPLATE(reduction_value, CTOR_NE_ND)

    J_A(AI, ND) explicit inline reduction_value(term_expr * J_NOT_NULL expr) noexcept : expr(expr) { }

    J_A(AI,ND) inline reduction_value(term_expr * J_NOT_NULL expr, enum rewrite_action action, i8_t rewrite_offset) noexcept
      : expr(expr),
        rewrite_action(action),
        rewrite_offset(rewrite_offset)
    { }

    J_A(AI) inline bool operator!() const noexcept { return !expr; }
    J_A(AI) explicit inline operator bool() const noexcept { return expr; }

    reduction_value(const reduction_value & rhs) noexcept;
    reduction_value(const reduction_value & rhs, copy_cb_t callback, void * userdata) noexcept;
    reduction_value & operator=(const reduction_value & rhs) noexcept;

    [[nodiscard]] bool operator==(const reduction_value & rhs) const noexcept;

    J_A(ND) reduction_value(reduction_value && rhs) noexcept
      : expr(rhs.expr),
        rewrite_action(rhs.rewrite_action),
        rewrite_offset(rhs.rewrite_offset)
    {
      rhs.expr = nullptr;
    }

    reduction_value & operator=(reduction_value && rhs) noexcept {
      if (J_LIKELY(this != &rhs)) {
        expr = rhs.expr;
        rewrite_action = rhs.rewrite_action;
        rewrite_offset = rhs.rewrite_offset;
        rhs.expr = nullptr;
      }
      return *this;
    }
  };

  using reduction_args = vector<reduction_value>;

  struct reduction_value_hash final  {
    [[nodiscard]] u32_t operator()(const reduction_value & r) const noexcept;
  };

  struct pat_captures final {
    vector<pat_capture> captures;
    i32_t num_used = 0;
    pat_capture this_capture;

    J_A(AI,NODISC,ND) inline u32_t size() const noexcept { return num_used; }

    void set(i8_t index, const pat_capture & c, bool overwrite = false) noexcept;

    J_A(AI,ND,HIDDEN) inline void set(arg_type type, i8_t index, i8_t stack_offset, val_type * value_type = nullptr, nt_p nt = nullptr, u8_t field_offset = 0) noexcept {
      set(index, pat_capture{type, stack_offset, value_type, nt, field_offset });
    }

    J_A(AI,ND,HIDDEN) inline void overwrite(arg_type type, i8_t index, i8_t stack_offset, val_type * value_type = nullptr, nt_p nt = nullptr, u8_t field_offset = 0) noexcept {
      set(index, pat_capture{type, stack_offset, value_type, nt, field_offset }, true);
    }


    J_A(AI,NODISC,ND,HIDDEN) inline bool operator!() const noexcept {
      return !num_used && !this_capture;
    }
    J_A(AI,NODISC,ND,HIDDEN) inline explicit operator bool() const noexcept {
      return num_used || this_capture;
    }
    [[nodiscard]] bool operator==(const pat_captures &) const noexcept;

    J_A(AI,NODISC) inline pat_capture * maybe_at(i32_t i) noexcept {
      if (i == -1) {
        return this_capture ? &this_capture : nullptr;
      }
      return i < (i32_t)captures.size() && captures[i] ? &captures[i] : nullptr;
    }
    J_A(AI,NODISC) inline const pat_capture * maybe_at(i32_t i) const noexcept {
      if (i == -1) {
        return this_capture ? &this_capture : nullptr;
      }
      return i < (i32_t)captures.size() && captures[i] ? &captures[i] : nullptr;
    }

    J_A(AI,NODISC,HIDDEN) inline pat_capture & operator[](i32_t i) noexcept {
      if (i == -1) {
        return this_capture;
      }
      return captures[i];
    }
    J_A(AI,NODISC,HIDDEN) inline const pat_capture & operator[](i32_t i) const noexcept {
      if (i == -1) {
        return this_capture;
      }
      return captures[i];
    }
    [[nodiscard]] pat_capture & at(i32_t i);

    J_A(AI,NODISC,HIDDEN) inline const pat_capture & at(i32_t i) const {
      return const_cast<pat_captures*>(this)->at(i);
    }

    J_A(AI,NODISC) inline pat_capture * begin() noexcept { return captures.begin(); }
    J_A(AI,NODISC) inline const pat_capture * begin() const noexcept { return captures.begin(); }
    J_A(AI,NODISC) inline pat_capture * end() noexcept { return captures.end(); }
    J_A(AI,NODISC) inline const pat_capture * end() const noexcept { return captures.end(); }

    static const pat_captures empty;
  };

  struct reduction_arg_hash final  {
    [[nodiscard]] u32_t operator()(const reduction_arg & r) const noexcept;
  };
}
