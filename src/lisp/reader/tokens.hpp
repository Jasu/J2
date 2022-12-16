#pragma once

#include "lisp/common/id.hpp"
#include "lisp/sources/source_location.hpp"

namespace j::lisp::inline values {
  struct lisp_str;
}
namespace j::lisp::reader {
  enum class token_type : char {
    open_paren = '(',
    close_paren = ')',
    open_bracket = '[',
    close_bracket = ']',
    quote = '\'',
    quasiquote = '`',
    unquote = ',',
    /// "some-local-symbol" or ":some-keyword"
    symbol = 'S',
    /// true or false
    boolean = 'b',
    /// nil
    nil = 'n',
    /// Double-quoted string
    string = '"',
    integer = 'i',
    floating_point = 'f',
  };

  struct open_paren_token;
  struct close_paren_token;
  struct open_bracket_token;
  struct close_bracket_token;
  struct quote_token;
  struct quasiquote_token;
  struct unquote_token;
  struct symbol_token;
  struct boolean_token;
  struct nil_token;
  struct string_token;
  struct integer_token;
  struct floating_point_token;
#define J_TOKEN_CASE(NAME) \
      case token_type::NAME: \
        return static_cast<Fn &&>(fn)(*reinterpret_cast<const NAME ## _token *>(this), static_cast<Args &&>(args)...)

  struct token {
    template<typename Fn, typename... Args>
    decltype(auto) visit(Fn && fn, Args && ... args) const {
      switch (type) {
        J_TOKEN_CASE(close_paren);
        J_TOKEN_CASE(open_paren);
        J_TOKEN_CASE(close_bracket);
        J_TOKEN_CASE(open_bracket);
        J_TOKEN_CASE(quote);
        J_TOKEN_CASE(quasiquote);
        J_TOKEN_CASE(unquote);
        J_TOKEN_CASE(string);
        J_TOKEN_CASE(boolean);
        J_TOKEN_CASE(nil);
        J_TOKEN_CASE(symbol);
        J_TOKEN_CASE(integer);
        J_TOKEN_CASE(floating_point);
      }
    }
    token_type type;
    bool has_pre_ellipsis:1 = false;
    bool has_post_ellipsis:1 = false;
  };

  struct open_token : token {
    inline explicit open_token(token_type type, open_token * parent, sources::source_location cur_loc) noexcept
      : token{.type = type},
        parent(parent),
        source_location(cur_loc)
    { }

    open_token * parent = nullptr;
    token * close_token = nullptr;
    u32_t size = 0U;
    sources::source_location source_location;
  };

  struct open_paren_token final : open_token {
    inline explicit open_paren_token(open_token * parent, sources::source_location cur_loc,
                                        bool has_pre_ellipsis = false) noexcept
      : open_token(token_type::open_paren, parent, cur_loc)
    {
      this->has_pre_ellipsis = has_pre_ellipsis;
    }
  };

  struct open_bracket_token final : open_token {
    inline explicit open_bracket_token(open_token * parent, sources::source_location cur_loc) noexcept
      : open_token(token_type::open_bracket, parent, cur_loc)
    { }
  };

  struct close_paren_token final : public token {
    inline close_paren_token() noexcept
    : token{token_type::close_paren} { }
  };

  struct close_bracket_token final : public token {
    inline close_bracket_token() noexcept
    : token{token_type::close_bracket} { }
  };

  struct quote_token final : public token {
    inline quote_token() noexcept : token{token_type::quote} { }
  };

  struct quasiquote_token final : public token {
    inline quasiquote_token() noexcept : token{token_type::quasiquote} { }
  };

  struct unquote_token final : public token {
    inline unquote_token() noexcept : token{token_type::unquote} { }
  };

  struct symbol_token final : public token {
    struct id id;
    inline symbol_token(struct id id, bool has_pre_ellipsis = false) noexcept
      : token{token_type::symbol, has_pre_ellipsis},
        id(id)
    { }
  };


  struct string_token final : public token{
    lisp_str * value;
    explicit string_token(lisp_str * J_NOT_NULL value) noexcept
      : token{token_type::string},
        value(value)
    {
    }
  };

  struct nil_token final : public token{
    explicit nil_token() noexcept
      : token{token_type::nil}
    {
    }
  };

  struct boolean_token final : public token{
    bool value;
    explicit boolean_token(bool value) noexcept
      : token{token_type::boolean},
        value(value)
    {
    }
  };

  struct integer_token final : public token {
    i64_t value;
    inline explicit integer_token(i64_t value) noexcept
      : token{token_type::integer},
        value(value)
    { }
  };

  struct floating_point_token final : public token {
    float value;
    inline explicit floating_point_token(float value) noexcept
      : token{token_type::floating_point},
        value(value)
    { }
  };

  inline u32_t token_length(const token * J_NOT_NULL t) noexcept {
    switch (t->type) {
    case token_type::close_paren:
    case token_type::close_bracket:
    case token_type::quote:
    case token_type::quasiquote:
    case token_type::unquote:
      return sizeof(token);
    case token_type::open_paren:
      return sizeof(open_paren_token);
    case token_type::open_bracket:
      return sizeof(open_bracket_token);
    case token_type::nil:
      return sizeof(nil_token);
    case token_type::boolean:
      return sizeof(boolean_token);
    case token_type::string:
      return sizeof(string_token);
    case token_type::symbol:
      return sizeof(symbol_token);
    case token_type::integer:
      return sizeof(integer_token);
    case token_type::floating_point:
      return sizeof(floating_point_token);
    }
  }

  inline const token * next_token(const token * J_NOT_NULL J_NOT_NULL t) noexcept {
    return add_bytes(t, token_length(t));
  }
}
