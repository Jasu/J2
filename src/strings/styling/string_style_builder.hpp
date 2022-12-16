#pragma once

#include "strings/styling/string_style.hpp"
#include "containers/deque_fwd.hpp"

J_DECLARE_EXTERN_DEQUE(j::strings::style);

namespace j::strings {
  inline namespace styling {
    class string_style_builder final {
    public:
      void push_style(u32_t at, style s);
      void pop_style(u32_t at);
      string_style && build() noexcept;
    private:
      deque<style> m_stack;
      string_style m_result;
      style m_current;
    };

    inline void string_style_builder::push_style(u32_t at, style s) {
      override_style_inline(m_current, s);
      m_stack.push_back(m_current);
      m_result.push_back(string_style_segment(at, m_current));
    }

    inline void string_style_builder::pop_style(u32_t at) {
      m_stack.pop_back();
      m_current = m_stack.empty() ? style() : m_stack.back();
      m_result.push_back(string_style_segment(at, m_current));
    }

    inline string_style && string_style_builder::build() noexcept {
      return static_cast<string_style &&>(m_result);
    }
  }
}
