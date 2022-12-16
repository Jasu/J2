#pragma once

#include "meta/attr_value.hpp"
#include "meta/code_writer.hpp"

namespace j::meta::inline rules {
  struct pattern;
  struct nonterminal_pattern;
}
namespace j::meta {
  struct module;
  struct expr;
  struct attrs;

  enum node_set_style : u8_t {
    node_set_ctor,
    node_set_mask,
  };

  struct dump_context_state {
    u8_t precedence = 0U;
    node_set_style set_style = node_set_ctor;
    strings::style set_color{};
  };

  struct dump_context_guard final {
    dump_context_state * ctx = nullptr;
    dump_context_state state;

    J_A(AI,ND) inline void restore() noexcept {
      if (ctx) {
        *ctx = state;
        ctx = nullptr;
      }
    }

    J_A(AI,ND) inline ~dump_context_guard() {
      restore();
    }
  };

  struct dump_context : dump_context_state {
    code_writer & to;
    module * mod = nullptr;

    void write_header(const char * J_NOT_NULL  title, strings::style s) noexcept;
    void write_index(u32_t index, u8_t pad, strings::style s) noexcept;
    void write_number(i64_t num, strings::style s = {}) noexcept;

    J_A(AI,ND,NODISC) inline dump_context_guard enter_node_set(node_set_style style, strings::style s = strings::style{}) noexcept {
      dump_context_guard result{this, *this};
      set_style = style;
      if (!s.empty()) {
        set_color = s;
      }
      return result;
    }

    J_A(AI,ND) inline u8_t set_precedence(u8_t prec) noexcept {
      u8_t res = precedence;
      precedence = prec;
      return res;
    }

    J_A(AI,ND) inline u8_t enter_call_like() noexcept {
      u8_t res = precedence;
      precedence = 0U;
      return res;
    }

    J_A(AI,ND) inline void exit_call_like(u8_t prec) noexcept {
      precedence = prec;
    }

    u8_t enter_precedence(u8_t prec) noexcept;
    void exit_precedence(u8_t prec) noexcept;

    J_A(AI,ND) inline dump_context(code_writer & to, module * J_NOT_NULL mod) noexcept
      : to(to),
        mod(mod)
    { }
  };

  struct dump_context_str_impl;
  struct dump_context_str : dump_context {
    explicit dump_context_str(module * J_NOT_NULL mod);
    ~dump_context_str();
    [[nodiscard]] strings::string build() noexcept;
    dump_context_str_impl * impl = nullptr;
  private:
    explicit dump_context_str(dump_context_str_impl * J_NOT_NULL impl, module * J_NOT_NULL mod);
  };

  void dump(dump_context & ctx, const codegen_template & tpl) noexcept;
  void dump(dump_context & ctx, const attr_value & value) noexcept;
  void dump(dump_context & ctx, const wrapped_node_set & value) noexcept;
  void dump(dump_context & ctx, const expr & value) noexcept;
  void dump(dump_context & ctx, const term & term) noexcept;
  void dump(dump_context & ctx, const pattern & value, const nonterminal * nt = nullptr) noexcept;
  void dump(dump_context & ctx, const attrs & attrs, bool space = false) noexcept;
  [[nodiscard]] strings::style get_placeholder_style(u8_t index) noexcept;
  void dump_placeholder(dump_context & ctx, u8_t index, bool is_seq, const nonterminal * nt = nullptr) noexcept;

  template<typename T>
  J_A(AI,ND) inline void dump(strings::formatted_sink & to, module * J_NOT_NULL mod, const T & v) noexcept {
    code_writer wr(&to);
    dump_context ctx(wr, mod);
    dump(ctx, v);
  }

  template<typename... Ts>
  strings::string dump_str(module * mod, const Ts & ... vs) noexcept {
    dump_context_str ctx(mod);
    dump(ctx, vs...);
    return ctx.build();
  }
}
namespace j::meta::inline rules {
  void dump(dump_context & ctx, const nonterminal & value) noexcept;
}
