#include "ansi/sgr_styled_sink.hpp"
#include "mem/shared_ptr.hpp"
#include "exceptions/assert_lite.hpp"
#include "ansi/sgr.hpp"
#include "strings/styling/style.hpp"
#include "services/service_definition.hpp"
#include "services/service_instance.hpp"
#include "strings/attributes.hpp"
#include "hzd/string.hpp"
#include "streams/fd_sink.hpp"

namespace j::ansi {
  sgr_styled_sink::sgr_styled_sink(mem::shared_ptr<streams::sink> sink) noexcept
    : m_write_ptr(m_buffer),
      m_sink(static_cast<mem::shared_ptr<streams::sink> &&>(sink))
  {
    J_ASSERT_NOT_NULL(m_sink);
  }

  u32_t sgr_styled_sink::write(const char * J_NOT_NULL from, u32_t num_bytes) noexcept {
    column += num_bytes;

    u32_t avail = buffer_free();

    // Check if the write fits in the buffer.
    if (num_bytes <= avail) {
      ::j::memcpy(m_write_ptr, from, num_bytes);
      m_write_ptr += num_bytes;
      return num_bytes;
    }

    const u32_t result = num_bytes;
    // If the buffer has data, fill it and flush it.
    ::j::memcpy(m_write_ptr, from, avail);
    num_bytes -= avail;
    from += avail;
    m_write_ptr += avail;
    flush();

    // Write past the buffer until the source string fits in the buffer.
    while (num_bytes >= buffer_size_v) {
      const u32_t written = m_sink->write(from, num_bytes);
      num_bytes -= written;
      from += written;
    }

    if (num_bytes) {
      ::j::memcpy(m_buffer, from, num_bytes);
      m_write_ptr = m_buffer + num_bytes;
    }

    return result;
  }

  J_RETURNS_NONNULL char * sgr_styled_sink::get_write_buffer(u32_t num_bytes) {
    column += num_bytes;
    J_ASSERT_NOT_NULL(m_sink);
    J_ASSUME(num_bytes <= buffer_size_v);
    if (buffer_free() < num_bytes) {
      flush();
    }
    char * const result = m_write_ptr;
    m_write_ptr += num_bytes;
    return result;
  }

  void sgr_styled_sink::flush() {
    J_ASSERT_NOT_NULL(m_sink, m_buffer);
    const char * from = m_buffer;
    const char * const end = m_write_ptr;
    while (from != end) {
      from += m_sink->write(from, end - from);
    }
    m_write_ptr = m_buffer;
  }

  bool sgr_styled_sink::is_tty() const {
    if (!m_has_is_tty) {
      m_has_is_tty = true;
      m_is_tty = m_sink->is_tty();
    }
    return m_is_tty;
  }

  static char * put_sgr_truecolor(char * J_NOT_NULL ptr, bool is_bg, const colors::rgb8 & color) noexcept {
    ::j::memcpy(ptr, is_bg ? "48;2;" : "38;2;", 5U);
    ptr += 5U;
    ptr = strings::formatting::format_dec((u8_t)color.r, ptr);
    *ptr++ = ';';
    ptr = strings::formatting::format_dec((u8_t)color.g, ptr);
    *ptr++ = ';';
    ptr = strings::formatting::format_dec((u8_t)color.b, ptr);
    return ptr;
  }

  static char * put_sgr_color(char * J_NOT_NULL ptr, bool is_bg, const colors::color color) noexcept {
    if (color.has_rgb) {
      return put_sgr_truecolor(ptr, is_bg, color.rgb);
    } else if (color.has_ansi) {
      return put_sgr_cmd(ptr, is_bg ? bg_color_to_sgr(color.ansi)
                         : fg_color_to_sgr(color.ansi));
    } else {
      *ptr++ = is_bg ? '4' : '3';
      *ptr++ = '9';
      return ptr;
    }
  }

  void sgr_styled_sink::set_style(const strings::style & style) {
    J_ASSERT_NOT_NULL(m_buffer, m_sink);
    u8_t set_flags = (style.flags.value & 0xF & ~m_current_style.flags.value);
    u8_t clear_flags = (m_current_style.flags.value & 0xF & ~style.flags.value);
    bool fg_eq = m_current_style.foreground == style.foreground;
    bool bg_eq = m_current_style.background == style.background;
    if (!set_flags && !clear_flags && fg_eq && bg_eq) {
      return;
    }
    m_current_style = style;
    if (!is_tty()) {
      return;
    }
    if (buffer_free() < 2 + 3 * 4 + 17 * 2) {
      flush();
    }
    *m_write_ptr++ = '\e';
    *m_write_ptr++ = '\[';
    bool has_written = false;
    for (u8_t flag = 1; flag < 16; flag <<= 1) {
      if ((clear_flags & flag) || (set_flags & flag)) {
        if (has_written) {
          *m_write_ptr++ = ';';
        }
        if (clear_flags & flag) {
          *m_write_ptr++ = '2';
        }
        switch (flag) {
        case 1:
          *m_write_ptr++ = (clear_flags & flag) ? '2' : '1';
          break;
        case 2:
          *m_write_ptr++ = '3';
          break;
        case 4:
          *m_write_ptr++ = '9';
          break;
        case 8:
          *m_write_ptr++ = '4';
          break;
        }
        has_written = true;
      }
    }
    if (!fg_eq) {
      if (has_written) {
        *m_write_ptr++ = ';';
      }
      m_write_ptr = put_sgr_color(m_write_ptr, false, style.foreground);
      has_written = true;
    }
    if (!bg_eq) {
      if (has_written) {
        *m_write_ptr++ = ';';
      }
      m_write_ptr = put_sgr_color(m_write_ptr, true, style.background);
    }
    *m_write_ptr++ = 'm';
  }

  strings::style sgr_styled_sink::get_style() const {
    return m_current_style;
  }

  u32_t sgr_styled_sink::write_styled(strings::styling::replace_style_tag,
                                      const strings::style & style,
                                      const char * from, u32_t num_bytes)
  {
    if (style == m_current_style) {
      return write(from, num_bytes);
    }
    strings::style old = m_current_style;
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

  sgr_styled_sink::~sgr_styled_sink() {
    if (m_sink) {
      flush();
    }
  }

  u32_t sgr_styled_sink::get_column() noexcept {
    return column;
  }

  void sgr_styled_sink::reset_column() noexcept {
    column = 0U;
  }

  void sgr_styled_sink::pad_to_column(u32_t i) {
    for (u32_t j = column; j < i; ++j) {
      write(" ", 1);
    }
  }

  J_RETURNS_NONNULL const char * sgr_styled_sink::buffer_end() const noexcept {
    return &m_buffer[buffer_size_v];
  }

  u32_t sgr_styled_sink::buffer_free() const noexcept {
    return buffer_end() - m_write_ptr;
  }

  namespace {
    namespace s = services;
    J_A(ND, NODESTROY) const s::service_definition<sgr_styled_sink> def(
      "ansi.sgr_styled_sink",
      "ANSI SGR Styled Sink",
      s::implements = s::interface<streams::sink>,
      s::implements = s::interface<strings::styling::styled_sink>,
      s::no_default_instance,
      s::create = s::constructor<mem::shared_ptr<streams::sink>>(
        s::arg::service_attribute(strings::attributes::sink)));

    J_A(ND, NODESTROY) const s::service_instance<sgr_styled_sink> stdout_def(
      "files.styled_stdout",
      "ANSI SGR Styled Standard Output",
      strings::attributes::sink = s::service<streams::fd_sink>("files.stdout"));

    J_A(ND, NODESTROY) const s::service_instance<sgr_styled_sink> stderr_def(
      "files.styled_stderr",
      "ANSI SGR Styled Standard Error",
      strings::attributes::sink = s::service<streams::fd_sink>("files.stderr"));
  }
}
