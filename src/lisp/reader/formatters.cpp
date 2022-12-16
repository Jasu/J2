#include "strings/formatting/formatter.hpp"
#include "strings/formatters/enum_formatter.hpp"
#include "strings/formatters/strings.hpp"
#include "strings/formatters/numbers.hpp"
#include "strings/styling/styled_sink.hpp"
#include "lisp/env/name_table.hpp"
#include "lisp/env/context.hpp"
#include "lisp/env/debug_formatters.hpp"
#include "lisp/reader/state.hpp"

namespace j::lisp::reader {
  namespace {
    namespace s = j::strings;
    namespace f = s::formatters;

    J_A(ND, NODESTROY) const f::enum_formatter<token_type> token_type_formatter{{
      {token_type::open_paren,       "Open paren \"(\""},
      {token_type::close_paren,      "Close paren \")\""},
      {token_type::open_bracket,       "Open bracket \"[\""},
      {token_type::close_bracket,      "Close bracket \"]\""},
      {token_type::quote,            "Quote \"'\""},
      {token_type::quasiquote,       "Quasi-quote \"`\""},
      {token_type::unquote,          "Unquote \",\""},
      {token_type::symbol,           "Symbol"},
      {token_type::string,           "String"},
      {token_type::integer,          "Integer"},
      {token_type::nil,              "Nil"},
      {token_type::boolean,          "Bool"},
      {token_type::floating_point,   "Float"},
    }};

    template<typename T>
    class J_TYPE_HIDDEN simple_token_formatter final : public s::formatter_known_length<T> {
      public:
      void do_format(
        const s::const_string_view & format_options,
        const T & value,
        s::styled_sink & target,
        s::style init_style
      ) const override {
        return token_type_formatter.do_format(format_options, value.type, target, init_style);
      }

      [[nodiscard]] u32_t do_get_length(const s::const_string_view & fo,
                          const T & v) const noexcept override
      {
        return token_type_formatter.do_get_length(fo, v.type);
      }
    };

    J_A(ND, NODESTROY) const simple_token_formatter<open_paren_token> g_open_paren_formatter;
    J_A(ND, NODESTROY) const simple_token_formatter<close_paren_token> g_close_paren_formatter;
    J_A(ND, NODESTROY) const simple_token_formatter<open_bracket_token> g_open_bracket_formatter;
    J_A(ND, NODESTROY) const simple_token_formatter<close_bracket_token> g_close_bracket_formatter;
    J_A(ND, NODESTROY) const simple_token_formatter<quote_token> g_quote_formatter;
    J_A(ND, NODESTROY) const simple_token_formatter<quasiquote_token> g_quasiquote_formatter;
    J_A(ND, NODESTROY) const simple_token_formatter<unquote_token> g_unquote_formatter;

    class J_TYPE_HIDDEN symbol_token_formatter final : public s::formatter_known_length<symbol_token> {
      public:
      void do_format(
        const s::const_string_view & fo,
        const symbol_token & value,
        s::styled_sink & target,
        s::style init_style
      ) const override {
        target.write("Symbol ");
        env::g_id_formatter.do_format(fo, value.id, target, init_style);
      }

      [[nodiscard]] u32_t do_get_length(const s::const_string_view & fo,
                          const symbol_token & value) const noexcept override
      {
        return 7 +
        env::g_id_formatter.do_get_length(fo, value.id);
      }
    };

    J_A(ND, NODESTROY) const symbol_token_formatter g_symbol_formatter;

    class J_TYPE_HIDDEN boolean_token_formatter final : public s::formatter_known_length<boolean_token> {
      public:
      void do_format(
        const s::const_string_view &,
        const boolean_token & value,
        s::styled_sink & target,
        s::style
      ) const override {
        target.write(value.value ? "True" : "False");
      }
      [[nodiscard]] u32_t do_get_length(const s::const_string_view &,
                          const boolean_token & v) const noexcept override
      {
        return v.value ? 4U : 5U;
      }
    };

    J_A(ND, NODESTROY) const boolean_token_formatter g_boolean_formatter;

    class J_TYPE_HIDDEN nil_token_formatter final : public s::formatter_known_length<nil_token> {
      public:
      void do_format(
        const s::const_string_view &,
        const nil_token &,
        s::styled_sink & target,
        s::style
      ) const override {
        target.write("Nil");
      }

      [[nodiscard]] u32_t do_get_length(const s::const_string_view &,
                          const nil_token &) const noexcept override
      {
        return 3;
      }
    };

    J_A(ND, NODESTROY) const nil_token_formatter g_nil_formatter;

    class J_TYPE_HIDDEN string_token_formatter final : public s::formatter_known_length<string_token> {
      public:
      void do_format(
        const s::const_string_view & format_options,
        const string_token & value,
        s::styled_sink & target,
        s::style init_style
      ) const override {
        target.write("String \"");
        f::g_const_string_view_formatter.do_format(format_options, value.value->value(), target, init_style);
        target.write("\"");
      }
      [[nodiscard]] u32_t do_get_length(const s::const_string_view & fo,
                          const string_token & v) const noexcept override
      {
        return 9 + f::g_const_string_view_formatter.do_get_length(fo, v.value->value());
      }
    };

    J_A(ND, NODESTROY) const string_token_formatter g_string_formatter;

    class J_TYPE_HIDDEN int_token_formatter final : public s::formatter_known_length<integer_token> {
      public:
      void do_format(
        const s::const_string_view & format_options,
        const integer_token & value,
        s::styled_sink & target,
        s::style init_style
      ) const override {
        target.write("Integer ");
        f::integer_formatter_v<i64_t>.do_format(format_options, value.value, target, init_style);
      }

      [[nodiscard]] u32_t do_get_length(const s::const_string_view & fo,
                          const integer_token & v) const noexcept override
      {
        return 8 + f::integer_formatter_v<i64_t>.do_get_length(fo, v.value);
      }
    };

    J_A(ND, NODESTROY) const int_token_formatter g_int_token_formatter;

    class J_TYPE_HIDDEN float_token_formatter final : public s::formatter_known_length<floating_point_token> {
      public:
      void do_format(
        const s::const_string_view & format_options,
        const floating_point_token & value,
        s::styled_sink & target,
        s::style init_style
      ) const override {
        target.write("Float ");
        f::float_formatter_v<double>.do_format(format_options, value.value, target, init_style);
      }

      [[nodiscard]] u32_t do_get_length(const s::const_string_view & fo,
                          const floating_point_token & v) const noexcept override
      {
        return 6 + f::float_formatter_v<double>.do_get_length(fo, v.value);
      }
    };

    J_A(ND, NODESTROY) const float_token_formatter g_float_token_formatter;

    template<typename Token> struct token_formatter_helper;

    template<> struct token_formatter_helper<open_paren_token>
    { static inline const auto & value = g_open_paren_formatter; };
    template<> struct token_formatter_helper<close_paren_token>
    { static inline const auto & value = g_close_paren_formatter; };
    template<> struct token_formatter_helper<open_bracket_token>
    { static inline const auto & value = g_open_bracket_formatter; };
    template<> struct token_formatter_helper<close_bracket_token>
    { static inline const auto & value = g_close_bracket_formatter; };
    template<> struct token_formatter_helper<quote_token>
    { static inline const auto & value = g_quote_formatter; };
    template<> struct token_formatter_helper<quasiquote_token>
    { static inline const auto & value = g_quasiquote_formatter; };
    template<> struct token_formatter_helper<unquote_token>
    { static inline const auto & value = g_unquote_formatter; };
    template<> struct token_formatter_helper<symbol_token>
    { static inline const auto & value = g_symbol_formatter; };
    template<> struct token_formatter_helper<boolean_token>
    { static inline const auto & value = g_boolean_formatter; };
    template<> struct token_formatter_helper<nil_token>
    { static inline const auto & value = g_nil_formatter; };
    template<> struct token_formatter_helper<string_token>
    { static inline const auto & value = g_string_formatter; };
    template<> struct token_formatter_helper<floating_point_token>
    { static inline const auto & value = g_float_token_formatter; };
    template<> struct token_formatter_helper<integer_token>
    { static inline const auto & value = g_int_token_formatter; };

    class J_TYPE_HIDDEN token_formatter final : public s::formatter_known_length<token> {
    public:
      void do_format(
        const s::const_string_view & fo,
        const token & value,
        s::styled_sink & target,
        s::style init_style
      ) const override {
        value.visit([&]<typename T>(const T & v) {
          token_formatter_helper<T>::value.do_format(fo, v, target, init_style);
        });
      }

      [[nodiscard]] u32_t do_get_length(const s::const_string_view & fo,
                          const token & value) const noexcept override
      {
        return value.visit([&]<typename T>(const T & v) {
          return token_formatter_helper<T>::value.do_get_length(fo, v);
        });
      }
    };

    J_A(ND, NODESTROY)const token_formatter g_token_formatter;

    class J_TYPE_HIDDEN state_formatter final : public s::formatter_known_length<state> {
    public:
      void do_format(
        const s::const_string_view & fo,
        const state & value,
        s::styled_sink & target,
        s::style init_style
      ) const override {
        for (auto & t : value) {
          g_token_formatter.do_format(fo, t , target, init_style);
          target.write("\n");
        }
      }

      [[nodiscard]] u32_t do_get_length(const s::const_string_view & fo, const state & value) const noexcept override
      {
        u32_t result = 0U;
        for (auto & t : value) {
          result += g_token_formatter.do_get_length(fo, t) + 1;
        }
        return result;
      }
    };

    J_A(ND, NODESTROY)const state_formatter g_state_formatter;
  }
}
