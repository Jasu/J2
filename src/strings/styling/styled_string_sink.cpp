#include "strings/styling/styled_string_sink.hpp"
#include "exceptions/assert_lite.hpp"
#include "hzd/string.hpp"

namespace j::strings {
  inline namespace styling {
    styled_string_sink::styled_string_sink(i32_t size)
      : m_string{strings::string{size}, {}},
        m_write_ptr(m_string.string.data())
    {
    }

    styled_string_sink::~styled_string_sink() {
    }

    u32_t styled_string_sink::write(const char * from, u32_t num_bytes) noexcept {
      m_column += num_bytes;
      if (num_bytes) {
        J_ASSERT_NOT_NULL(from);
        memcpy(m_write_ptr, from, num_bytes);
        m_write_ptr += num_bytes;
      }
      return num_bytes;
    }

    void styled_string_sink::flush() noexcept {
    }

    char * styled_string_sink::get_write_buffer(u32_t sz) {
      char * const result = m_write_ptr;
      m_column += sz;
      m_write_ptr += sz;
      return result;
    }

    void styled_string_sink::set_style(const style & style) noexcept {
      m_string.style.push_back(string_style_segment(m_write_ptr - m_string.string.data(), style));
    }

    style styled_string_sink::get_style() const {
      if (m_string.style.empty()) {
        return g_empty_style;
      }
      return m_string.style.back().style;
    }

    u32_t styled_string_sink::write_styled(replace_style_tag, const style & style, const char * from, u32_t num_bytes) {
      auto old = get_style();
      u32_t result;
      try {
        set_style(style);
        result = write(from, num_bytes);
      } catch (...) {
        set_style(old);
        throw;
      }
      set_style(old);
      return result;
    }

    u32_t styled_string_sink::get_column() noexcept {
      return m_column;
    }

    void styled_string_sink::reset_column() noexcept {
      m_column = 0U;
    }
    void styled_string_sink::pad_to_column(u32_t i) {
      for (u32_t j = m_column; j < i; ++j) {
        write(" ", 1);
      }
    }
  }
}
