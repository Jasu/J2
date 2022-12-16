#pragma once

#include "hzd/maybe.hpp"
#include "logging/global.hpp"
#include "strings/string.hpp"
#include "files/paths/path.hpp"
#include "meta/attr_value.hpp"
#include "meta/errors.hpp"
#include "meta/rules/common.hpp"

namespace j::meta::inline rules {
  struct pattern;
  struct pat_tree;
  struct term_expr;
  struct matcher_base;
  struct te_type;
  struct alias_table;
}

namespace j::meta {
  struct module;
  struct attr_type_def;
  struct doc_comment;

  struct base_parser_snapshot {
    const char * cursor = nullptr;
    const char * line_begin = nullptr;
    i32_t line_number = 1U;
  };

  struct base_parser_state {
    base_parser_state(const char * J_NOT_NULL at, const char * J_NOT_NULL end,
                      const files::path * J_NOT_NULL cur_path, module * J_NOT_NULL mod,
                      strings::const_string_view default_package, alias_table * aliases) noexcept;
    ~base_parser_state();

    const char * cursor = nullptr;
    const char * limit = nullptr;
    const char * line_begin = nullptr;
    i32_t line_number = 1U;
    alias_table * aliases = nullptr;
    module * const mod = nullptr;
    const files::path * cur_path = nullptr;
    i32_t log_enabled = 0;
    strings::const_string_view default_package;

    J_A(RNN,NODISC) term * term_at(strings::const_string_view name) const;
    J_A(RNN,NODISC) nt_p nt_at(strings::const_string_view name) const;
    J_A(NODISC) term_expr * term_expr_macro_maybe_at(strings::const_string_view name) const;
    J_A(NODISC) matcher_base * matcher_macro_maybe_at(strings::const_string_view name) const;

    [[nodiscard]] inline source_location loc() const noexcept {
      return source_location{
        (i32_t)line_number,
        (i32_t)(cursor - line_begin),
        cur_path,
      };
    }

    inline void enable_log() noexcept {
      log_enabled++;
    }
    inline void disable_log() noexcept {
      log_enabled--;
    }
    inline void newline() noexcept {
      ++line_number;
      line_begin = cursor;
    }

    template<typename... Ts>
    inline void log(const Ts & ... vs) const noexcept {
      if (log_enabled > 0) {
        J_DEBUG_INL(vs...);
        log_ctx();
      }
    }

    void log_ctx() const noexcept;

    [[noreturn]] void fail(strings::const_string_view msg, strings::const_string_view ctx = {}) const;

    J_A(AI) inline void check(bool pred, strings::const_string_view msg, strings::const_string_view ctx = {}) const {
      if (!pred) {
        fail(msg, ctx);
      }
    }

    [[nodiscard]] base_parser_snapshot snapshot() const noexcept {
      return {cursor, line_begin, line_number};
    }

    void restore(const base_parser_snapshot & snapshot) noexcept {
      cursor = snapshot.cursor;
      line_begin = snapshot.line_begin;
      line_number = snapshot.line_number;
    }
  };

  struct codegen_template;
  struct expr;

  [[nodiscard]] strings::const_string_view parse_to_eol(base_parser_state & state) noexcept;
  [[nodiscard]] i64_t read_dec(const char * J_NOT_NULL start, const char * J_NOT_NULL end) noexcept;
  [[nodiscard]] i64_t read_hex(const char * J_NOT_NULL start, const char * J_NOT_NULL end) noexcept;
  [[nodiscard]] u64_t read_bin(const char * J_NOT_NULL start, const char * J_NOT_NULL end) noexcept;
  J_A(RNN,NODISC) expr * parse_expr(base_parser_state & state);
  [[nodiscard]] strings::const_string_view parse_name_or_id(base_parser_state & state, strings::const_string_view ctx = "when parsing name or id.", bool allow_id = true, bool allow_name = true);
  [[nodiscard]] strings::const_string_view parse_name(base_parser_state & state, strings::const_string_view ctx = "when parsing name.");
  [[nodiscard]] strings::const_string_view parse_id(base_parser_state & state, strings::const_string_view ctx = "when parsing id.");
  [[nodiscard]] strings::string parse_string(base_parser_state & state);
  [[nodiscard]] strings::string parse_string_out(base_parser_state & state);
  [[nodiscard]] attr_value parse_tuple(base_parser_state & state);
  [[nodiscard]] attr_value parse_tuple_out(base_parser_state & state);
  [[nodiscard]] attr_value maybe_parse_tuple_out(base_parser_state & state);
  [[nodiscard]] codegen_template parse_template(base_parser_state & state);
  [[nodiscard]] codegen_template parse_template_out(base_parser_state & state);

  [[nodiscard]] attr_value parse_attr_value(base_parser_state & state, attr_type_def * type = nullptr, bool is_required = true);
  J_A(AI,ND,NODISC) inline attr_value maybe_parse_attr_value(base_parser_state & state, attr_type_def * type = nullptr) {
    return parse_attr_value(state, type, false);
  }

  [[nodiscard]] maybe<i64_t> maybe_parse_int(base_parser_state & state) noexcept;
  [[nodiscard]] i64_t parse_int(base_parser_state & state);

  [[nodiscard]] attr_type_def maybe_parse_attr_simple_type(base_parser_state & state, strings::const_string_view str) noexcept;
  [[nodiscard]] attr_type_def parse_attr_type(base_parser_state & state);
  char skip_ws(base_parser_state & state) noexcept;
  char skip_ws_up_to(base_parser_state & state, const char * J_NOT_NULL mask, const char * J_NOT_NULL ctx = "when skipping whitespace");
  char skip_ws_past(base_parser_state & state, const char * J_NOT_NULL mask, const char * J_NOT_NULL ctx);
  char skip_ws_past(base_parser_state & state, const char * J_NOT_NULL mask);
  [[nodiscard]] basic_node_set parse_term_set(base_parser_state & state);
  J_A(NODISC) rule * parse_rule(base_parser_state & state, i32_t index, nt_p reduce_as = nullptr);
  J_A(RNN,NODISC) term_expr * parse_term_expr(base_parser_state & state, nt_p this_nt = nullptr, te_type * parse_as = nullptr);
  enum class operator_type : u8_t;
  [[nodiscard]] operator_type parse_te_operator(base_parser_state & state);
}
