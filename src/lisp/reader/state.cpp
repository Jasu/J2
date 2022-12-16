#include "lisp/reader/state.hpp"
#include "lisp/env/environment.hpp"
#include "lisp/packages/pkg.hpp"
#include "exceptions/assert.hpp"

namespace j::lisp::reader {
  token_iterator::token_iterator(j::mem::bump_pool::const_iterator it)
      : m_it(it)
    {
      if (m_it) {
        m_cur = m_it->begin<token>();
        m_end = m_it->end<token>();
        J_ASSUME_NOT_NULL(m_cur, m_end);
        J_ASSUME(m_end >= m_cur);
        if (m_cur == m_end) {
          m_it.reset();
          m_cur = m_end = nullptr;
        }
      }
    }

  token_iterator & token_iterator::operator++() noexcept {
    J_ASSUME_NOT_NULL(m_cur, m_end);
    J_ASSUME(m_cur < m_end);
    m_cur = next_token(m_cur);
    J_ASSUME(m_cur <= m_end);
    if (m_cur == m_end) {
      if (++m_it) {
        m_cur = m_it->begin<token>();
        m_end = m_it->end<token>();
      } else {
        m_cur = m_end = nullptr;
      }
    }
    return *this;
  }

  state::state(env::environment * J_NOT_NULL env,
               packages::pkg * J_NOT_NULL pkg,
               const char * begin,
               u32_t size,
               u16_t source_index) noexcept
    : lex_state(begin, size, source_index),
      heap(&env->heap),
      pkg_current(pkg),
      pkg_global(env->packages[package_id_global]),
      pkg_keyword(env->packages[package_id_keyword]),
      env(env)
  {
    J_ASSUME_NOT_NULL(pkg_global, pkg_keyword);
  }

  void state::push_close_paren() {
    auto p = &m_pool.emplace<close_paren_token>();
    last_token = p;
    J_ASSERT(current_open && current_open->type == token_type::open_paren);
    current_open->close_token = p;
    current_open = current_open->parent;
  }

  void state::push_open_bracket(const char *cur_pos) {
    last_token = current_open = &m_pool.emplace<open_bracket_token>(current_open, lex_state.current_location(cur_pos));
  }

  void state::push_close_bracket() {
    auto p = &m_pool.emplace<close_bracket_token>();
    last_token = p;
    J_ASSERT(current_open && current_open->type == token_type::open_bracket);
    current_open->close_token = p;
    current_open = current_open->parent;
  }

  void state::push_token(const char *cur_pos, bool has_pre_ellipsis) {
    on_token();
    const char c = *cur_pos;
    if (c == '(') {
      current_open = &m_pool.emplace<open_paren_token>(current_open, lex_state.current_location(cur_pos));
      current_open->has_pre_ellipsis = has_pre_ellipsis;
      last_token = current_open;
    } else {
      is_expecting_single = c == '\'' || c == '`' || c == ',';
      last_token = &m_pool.emplace_braces<token>((token_type)c, has_pre_ellipsis);
    }
  }

  void state::push_integer(i64_t value) {
    on_token();
    last_token = &m_pool.emplace<integer_token>(value);
  }

  void state::push_floating_point(double value) {
    on_token();
    last_token = &m_pool.emplace<floating_point_token>(value);
  }

  void state::push_string(lisp_str * J_NOT_NULL str) {
    on_token();
    last_token = &m_pool.emplace<string_token>(str);
  }

  void state::on_token() noexcept {
    if (is_expecting_single) {
      is_expecting_single = false;
      return;
    }
    if (current_open) {
      current_open->size++;
    } else {
      ++num_root_elements;
    }
  }

  void state::push_symbol(strings::const_string_view package_name,
                          strings::const_string_view local_symbol,
                          bool is_keyword,
                          bool has_pre_ellipsis) {
    J_ASSERT(local_symbol && local_symbol != ":" && package_name != ":");
    on_token();
    id id;
    if (local_symbol == "true" || local_symbol == "false") {
      last_token = &m_pool.emplace<boolean_token>(local_symbol == "true");
      return;
    } else if (local_symbol == "nil") {
      last_token = &m_pool.emplace<nil_token>();
      return;
    } else if (is_keyword) {
      id = pkg_keyword->emplace_id(local_symbol);
    } else if (package_name) {
      J_ASSERT(package_name);
      id = pkg_current->emplace_unresolved_id(package_name, local_symbol);
    } else {
      id = pkg_global->symbol_table.names.try_get_id_of(local_symbol);
      if (!id) {
        id = pkg_current->emplace_id(local_symbol);
      }
    }
    last_token = &m_pool.emplace<symbol_token>(id, has_pre_ellipsis);
  }

  [[nodiscard]] token_iterator state::begin() const noexcept
  { return {m_pool.begin()}; }

  [[nodiscard]] token_iterator state::end() const noexcept { return {}; }

  [[noreturn]] void state::throw_error() {
    J_THROW("Unexpected input '{hex}'", strings::const_string_view(lex_state.cursor - 1, 1));
  }
}
