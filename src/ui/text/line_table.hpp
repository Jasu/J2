#pragma once

#include "ui/text/line.hpp"
#include "containers/vector.hpp"

namespace j::ui::inline text {
  struct line_table_entry final {
    line_t physical_line;
    vector<visual_line> visual_lines;

    using nicely_copyable_tag_t = void;
    using zero_initializable_tag_t = void;
  };
}

J_DECLARE_EXTERN_NONCOPYABLE_VECTOR(j::ui::text::line_table_entry);

namespace j::ui::inline text {
  class line_table final {
  public:
    J_INLINE_GETTER u32_t size() const noexcept { return m_end_line - m_begin_line; }
    J_INLINE_GETTER u64_t begin_line() const noexcept { return m_begin_line; }
    J_INLINE_GETTER u64_t end_line() const noexcept { return m_end_line; }
    J_ALWAYS_INLINE void clear() noexcept {
      m_table.clear();
      m_begin_line = m_end_line = 0U;
    }

    J_ALWAYS_INLINE void set_first_line_number(u64_t num) noexcept {
      m_begin_line += num;
      m_end_line += num;
    }

    J_INLINE_GETTER const line_table_entry * begin() const noexcept { return m_table.begin(); }
    J_INLINE_GETTER const line_table_entry * end() const noexcept   { return m_table.end(); }
    J_INLINE_GETTER line_table_entry * begin() noexcept             { return m_table.begin(); }
    J_INLINE_GETTER line_table_entry * end() noexcept               { return m_table.end(); }

    J_RETURNS_NONNULL line_table_entry * erase_line(u32_t index) noexcept;
    void remove_prefix(u32_t num) noexcept;
    void remove_suffix(u32_t num) noexcept;
    void prepend_line(line_table_entry && entry);
    void append_line(line_table_entry && entry);
  private:
    u64_t m_begin_line = 0U;
    u64_t m_end_line = 0U;
    using table_t = noncopyable_vector<line_table_entry>;
    table_t m_table;
  };
}
