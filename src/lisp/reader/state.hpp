#pragma once

#include "tokens.hpp"
#include "mem/bump_pool.hpp"
#include <lisp/reader/read_types.hpp>

namespace j::lisp::env {
  struct environment;
}

namespace j::lisp::packages {
  struct pkg;
}
namespace j::lisp::mem {
  class heap;
}

namespace j::lisp::reader {
  struct token_iterator final {
    j::mem::bump_pool::const_iterator m_it;
    const token * m_cur = nullptr;
    const token * m_end = nullptr;

    constexpr J_ALWAYS_INLINE token_iterator() noexcept = default;

    token_iterator(j::mem::bump_pool::const_iterator it);

    J_INLINE_GETTER_NONNULL const token * operator->() noexcept {
      return m_cur;
    }

    J_INLINE_GETTER const token & operator*() noexcept {
      return *m_cur;
    }

    token_iterator & operator++() noexcept;

    J_ALWAYS_INLINE token_iterator operator++(int) noexcept {
      token_iterator prev{*this};
      return operator++(), prev;
    }

    J_ALWAYS_INLINE bool operator==(const token_iterator & rhs) const noexcept = default;
  };

  struct state final {
    state(env::environment * J_NOT_NULL env,
          packages::pkg * J_NOT_NULL pkg,
          const char * J_NOT_NULL begin,
          u32_t size = 0U,
          u16_t source_index = 0U) noexcept;

    [[nodiscard]] token_iterator begin() const noexcept;

    [[nodiscard]] token_iterator end() const noexcept;

    void push_token(const char * J_NOT_NULL cur_pos, bool has_ellipsis = false);
    void push_close_paren();

    void push_open_bracket(const char *cur_pos);
    void push_close_bracket();

    void push_symbol(strings::const_string_view package_symbol,
                     strings::const_string_view local_symbol,
                     bool is_keyword,
                     bool has_pre_ellipsis);

    void push_integer(i64_t value);

    void push_floating_point(double value);

    void push_string(lisp_str * J_NOT_NULL str);

    void on_token() noexcept;

    lex_state lex_state;

    j::mem::bump_pool m_pool;
    open_token * current_open = nullptr;
    u32_t num_root_elements = 0U;
    bool is_expecting_single = false;
    mem::heap * heap = nullptr;
    token * last_token = nullptr;
    bool has_pre_ellipsis = false;

    [[nodiscard]] bool got_top_level() const noexcept {
      return !current_open && num_root_elements && !is_expecting_single;
    }

    [[nodiscard]] bool is_at_top_level() const noexcept {
      return !current_open && !is_expecting_single;
    }

    packages::pkg *pkg_current = nullptr, *pkg_global = nullptr, *pkg_keyword = nullptr;

    [[noreturn]] void throw_error();

    env::environment * env = nullptr;
  };
}
