#include "ui/text/line_table.hpp"

J_DEFINE_EXTERN_NONCOPYABLE_VECTOR(j::ui::text::line_table_entry);

namespace j::ui::inline text {
  void line_table::remove_prefix(u32_t num) noexcept {
    m_begin_line += num;
    m_table.pop_front(num);
  }

  J_RETURNS_NONNULL line_table_entry * line_table::erase_line(u32_t index) noexcept {
    --m_end_line;
    return m_table.erase(index);
  }

  void line_table::remove_suffix(u32_t num) noexcept {
    m_end_line -= num;
    m_table.pop_back(num);
  }

  void line_table::prepend_line(line_table_entry && entry) {
    m_table.push_front(static_cast<line_table_entry &&>(entry));
    --m_begin_line;
  }

  void line_table::append_line(line_table_entry && entry) {
    m_table.push_back(static_cast<line_table_entry &&>(entry));
    ++m_end_line;
  }
}
