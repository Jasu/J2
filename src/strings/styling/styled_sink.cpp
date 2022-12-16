#include "strings/styling/styled_sink.hpp"

#include "strings/styling/dummy_styled_sink.hpp"
#include "strings/styling/style.hpp"
#include "strings/styling/styled_string.hpp"
#include "hzd/string.hpp"
#include "services/interface_definition.hpp"

namespace j::strings {
  u32_t styled_sink::write_styled(const strings::style & style, const char * J_NOT_NULL from, u32_t num_bytes) {
    return write_styled(strings::styling::replace_style,
                        override_style(get_style(), style),
                        from, num_bytes);
  }

  u32_t styled_sink::write_styled(const styled_string & str) {
    u32_t sz = 0;
    for (auto & p : str) {
      sz += write_styled(strings::styling::replace_style, p.style(), p.string().data(), p.string().size());
    }
    return sz;
  }

  u32_t styled_sink::write_styled(const style & style, const styled_string & str) {
    const auto s = get_style();
    set_style(override_style(s, style));
    const u32_t r = write_styled(str);
    set_style(s);
    return r;
  }

  static constexpr sz_t g_buffer_size = 256UL;
  dummy_styled_sink::dummy_styled_sink() noexcept {
  }

  dummy_styled_sink::dummy_styled_sink(mem::shared_ptr<streams::sink> sink)
    : m_buffer(new char[g_buffer_size]),
      m_sink(static_cast<mem::shared_ptr<streams::sink> &&>(sink))
  {
  }

  dummy_styled_sink::dummy_styled_sink(dummy_styled_sink && rhs) noexcept
    : m_buffer(rhs.m_buffer),
      m_sink(static_cast<mem::shared_ptr<streams::sink> &&>(rhs.m_sink)),
      m_write_pos(rhs.m_write_pos),
      m_current_style(rhs.m_current_style)
  {
    rhs.m_buffer = nullptr;
  }

  dummy_styled_sink & dummy_styled_sink::operator=(dummy_styled_sink && rhs) noexcept {
    if (this != &rhs) {
      m_sink = static_cast<mem::shared_ptr<streams::sink> &&>(rhs.m_sink);
      m_buffer = rhs.m_buffer;
      m_write_pos = rhs.m_write_pos;
      rhs.m_buffer = nullptr;
      m_current_style = rhs.m_current_style;
    }
    return *this;
  }

  dummy_styled_sink::~dummy_styled_sink() {
    if (m_sink) {
      flush();
    }
    delete[] m_buffer;
  }

  u32_t dummy_styled_sink::write(const char * J_NOT_NULL from, u32_t num_bytes) noexcept {
    m_column += num_bytes;
    J_ASSERT_NOT_NULL(m_sink, m_buffer);

    // Check if the write fits in the buffer.
    if (g_buffer_size - m_write_pos >= num_bytes) {
      memcpy(m_buffer + m_write_pos, from, num_bytes);
      m_write_pos += num_bytes;
      return num_bytes;
    }

    const u32_t result = num_bytes;
    // If the buffer has data, fill it and flush it.
    if (m_write_pos) {
      const sz_t written = g_buffer_size - m_write_pos;
      memcpy(m_buffer + m_write_pos, from, written);
      from += written;
      num_bytes -= written;
      m_write_pos += written;
      flush();
    }

    // Write past the buffer until the source string fits in the buffer.
    while (num_bytes >= g_buffer_size) {
      const u32_t written = m_sink->write(from, num_bytes);
      num_bytes -= written;
      from += written;
    }

    if (num_bytes) {
      memcpy(m_buffer, from, num_bytes);
      m_write_pos = num_bytes;
    }

    return result;
  }

  char * dummy_styled_sink::get_write_buffer(u32_t num_bytes) {
    m_column += num_bytes;
    J_ASSERT_NOT_NULL(m_sink, m_buffer);
    if (g_buffer_size - m_write_pos < num_bytes) {
      flush();
    }
    J_ASSERT(num_bytes <= g_buffer_size, "TODO maximum buffer size exceeded.");
    m_write_pos += num_bytes;
    return m_buffer + m_write_pos - num_bytes;
  }

  void dummy_styled_sink::flush() {
    J_ASSERT_NOT_NULL(m_sink, m_buffer);
    const char * from = m_buffer;
    const char * end = m_buffer + m_write_pos;
    while (from != end) {
      from += m_sink->write(from, end - from);
    }
    m_write_pos = 0U;
  }

  bool dummy_styled_sink::is_tty() const {
    return m_sink && m_sink->is_tty();
  }

  void dummy_styled_sink::set_style(const style & style) noexcept {
    m_current_style = style;
  }

  style dummy_styled_sink::get_style() const noexcept {
    return m_current_style;
  }

  u32_t dummy_styled_sink::write_styled(styling::replace_style_tag,
                                        const strings::style &,
                                        const char * J_NOT_NULL from,
                                        u32_t num_bytes) {
    return write(from, num_bytes);
  }
  u32_t dummy_styled_sink::get_column() noexcept {
    return m_column;
  }

  void dummy_styled_sink::reset_column() noexcept {
    m_column = 0U;
  }

  void dummy_styled_sink::pad_to_column(u32_t i) {
    for (u32_t j = m_column; j < i; ++j) {
      write(" ", 1);
    }
  }

  u32_t styled_sink::write_styled(const style & style, strings::const_string_view str) {
    return write_styled(style, str.data(), str.size());
  }
  u32_t styled_sink::write_styled(const style & style, const char * J_NOT_NULL str) {
    return write_styled(style, str, ::j::strlen(str));
  }
  u32_t styled_sink::write_styled(replace_style_tag t, const style & style, strings::const_string_view str) {
    return write_styled(t, style, str.data(), str.size());
  }
  u32_t styled_sink::write_styled(replace_style_tag t, const style & style, const char * J_NOT_NULL str) {
    return write_styled(t, style, str, ::j::strlen(str));
  }

  inline namespace styling {
    namespace {
      J_A(ND, NODESTROY) const services::interface_definition<styled_sink> def("strings.styled_sink", "Styled Sink");
    }
  }
}
