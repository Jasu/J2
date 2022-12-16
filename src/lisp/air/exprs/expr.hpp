#pragma once

#include "lisp/air/exprs/input.hpp"
#include "lisp/air/exprs/expr_type.hpp"
#include "lisp/values/lisp_imm.hpp"
#include "containers/span.hpp"

namespace j::lisp::inline common{
  struct metadata_view;
  struct metadata_init;
}
namespace j::lisp::air::exprs {
  struct expr {
    expr_type type               = expr_type::nop;
    u8_t num_inputs              = 0U;
    u8_t max_inputs              = 0U;

    bool visit_flag:1            = false;

    /// Generic reordering barrier - side effects could do anything except modify non-bound lex vars.
    bool has_side_effects:1      = false;
    /// Writes to memory - cannot be reordered with other memory writes, reads, side-effects or NLX.
    ///
    /// \note Closure-bound lexical variables count as memory.
    bool writes_mem:1            = false;
    /// Reads from memory - cannot be reordered with memory writes or side-effects.
    ///
    /// \note Closure-bound lexical variables count as memory.
    bool reads_mem:1             = false;
    /// Writes to an unbound lexical variable - cannot be reordered with lex-reads or writes.
    bool writes_lex:1            = false;
    /// Reads from an unbound lexical variable - cannot be reordered with lex-reads or writes.
    bool reads_lex:1             = false;
    /// May exit non-locally, i.e. throw an exception. Cannot be reordered with memory writes, side-effects, or other NLXes.
    bool may_nlx:1               = false;
    /// May exit non-locally, compiler-generated and movable.
    ///
    /// Compiler-inserted type checks and possible OOM conditions are movable among each other.
    bool movable_nlx:1           = false;
    val_spec result;
    u8_t parent_pos              = 0U;
    /// If set, metadata (i.e. debug info) is allocated after the expr.
    bool has_metadata:1          = false;
    expr * parent                = nullptr;

    J_A(AI,NODISC,FLATTEN) inline enum rep rep() const noexcept {
      return result.rep();
    }

    J_A(AI,NODISC) inline bool is_const() const noexcept {
      return type == expr_type::ld_const;
    }

    [[nodiscard]] const lisp_imm & get_const() const noexcept;

    [[nodiscard]] inline input_span inputs() noexcept {
      return { reinterpret_cast<struct input*>(this + 1), num_inputs };
    }
    [[nodiscard]] inline const_input_span inputs() const noexcept {
      return { reinterpret_cast<const struct input*>(this + 1), num_inputs };
    }

    J_A(AI,ND,NODISC) inline struct input & input(i32_t i) noexcept {
      return reinterpret_cast<struct input*>(this + 1)[i];
    }

    J_A(AI,ND,NODISC) inline const struct input & input(i32_t i) const noexcept {
      return reinterpret_cast<const struct input*>(this + 1)[i];
    }

    J_A(AI,ND,NODISC) inline expr * in_expr(i32_t i) const noexcept {
      return reinterpret_cast<const struct input*>(this + 1)[i].expr;
    }

    J_A(AI,ND,NODISC,RNN) inline struct input * inputs_begin() noexcept {
      return reinterpret_cast<struct input*>(this + 1);
    }
    J_A(AI,ND,NODISC,RNN) inline struct input * inputs_end() noexcept {
      return reinterpret_cast<struct input*>(this + 1) + num_inputs;
    }
    J_A(AI,ND,NODISC,RNN) inline const struct input * inputs_begin() const noexcept {
      return reinterpret_cast<const struct input*>(this + 1);
    }
    J_A(AI,ND,NODISC,RNN) inline const struct input * inputs_end() const noexcept {
      return reinterpret_cast<const struct input*>(this + 1) + num_inputs;
    }

    J_A(AI,NODISC,RNN) inline void * static_begin() noexcept {
      return reinterpret_cast<struct input*>(this + 1) + max_inputs;
    }
    J_A(AI,NODISC,RNN) inline const void * static_begin() const noexcept {
      return reinterpret_cast<const struct input*>(this + 1) + max_inputs;
    }

    J_A(RNN) expr * set_input(u8_t index, expr * J_NOT_NULL e) noexcept;

    [[nodiscard]] metadata_view metadata() const noexcept;
    [[nodiscard]] u32_t depth() const noexcept;
  };

  void write_expr_metadata(expr * J_NOT_NULL to, const metadata_init & init) noexcept;
}
