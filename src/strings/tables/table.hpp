#pragma once

#include "strings/tables/table_cell.hpp"
#include "exceptions/assert_lite.hpp"

namespace j::strings::inline tables {
  class table_row;
  class table_column;
  class table_row_iterator;
  class table_column_iterator;
  class column_cell_iterator;
  struct table_row_border_iterator;
  struct table_column_border_iterator;

  struct table_row_borders {
    table_row_border_iterator begin() const noexcept;
    table_row_border_iterator end() const noexcept;
    u32_t size() const noexcept;
  };

  struct table_column_borders {
    table_column_border_iterator begin() const noexcept;
    table_column_border_iterator end() const noexcept;
    u32_t size() const noexcept;
  };

  struct table_rows {
    table_row_iterator begin() const noexcept;
    table_row_iterator end() const noexcept;
    table_row operator[](u32_t i) const noexcept;
    u32_t size() const noexcept;
  };

  struct table_columns {
    table_column_iterator begin() const noexcept;
    table_column_iterator end() const noexcept;
    table_column operator[](u32_t i) const noexcept;
    u32_t size() const noexcept;
  };

  constexpr border_style combine_border_style(border_style a, border_style b, border_style def) noexcept {
    if (a == border_style::none || b == border_style::none) {
      return border_style::none;
    }
    if (a != border_style::table_default) {
      return a;
    }
    if (b != border_style::table_default) {
      return b;
    }
    J_ASSERT(def != border_style::table_default, "Default style cannot be default.");
    return def;
  }

  class table final : private table_rows, private table_columns {
  public:
    constexpr table() noexcept = default;
    table(u8_t width, u8_t height, const table_cell * cells)
      : m_width(width),
        m_height(height),
        m_cells(cells)
    {
      J_ASSERT((!m_width && !m_height && !m_cells) || (m_width && m_height && m_cells),
               "Either none or all of width, height, and cells must be given.");
    }

    template<typename... Attrs, typename = j::attributes::enable_if_attributes_t<Attrs...>>
    table(u8_t width, u8_t height, const table_cell * cells, Attrs && ... attrs)
      : table(width, height, cells)
    {
      namespace a = j::attributes;
      if constexpr (a::has<Attrs...>(border)) {
        m_border_style_top = m_border_style_bottom = m_border_style_left = m_border_style_right
          = m_border_style_vertical = m_border_style_horizontal
          = border.get(static_cast<Attrs &&>(attrs)...);
      }
      if constexpr (a::has<Attrs...>(border_top)) {
        m_border_style_top = border_top.get(static_cast<Attrs &&>(attrs)...);
      }
      if constexpr (a::has<Attrs...>(border_bottom)) {
        m_border_style_bottom = border_bottom.get(static_cast<Attrs &&>(attrs)...);
      }
      if constexpr (a::has<Attrs...>(border_left)) {
        m_border_style_left = border_left.get(static_cast<Attrs &&>(attrs)...);
      }
      if constexpr (a::has<Attrs...>(border_right)) {
        m_border_style_right = border_right.get(static_cast<Attrs &&>(attrs)...);
      }
      if constexpr (a::has<Attrs...>(border_vertical)) {
        m_border_style_vertical = border_vertical.get(static_cast<Attrs &&>(attrs)...);
      }
      if constexpr (a::has<Attrs...>(border_horizontal)) {
        m_border_style_horizontal = border_horizontal.get(static_cast<Attrs &&>(attrs)...);
      }
    }

    const table_rows & rows() const noexcept {
      return *this;
    }

    const table_columns & columns() const noexcept {
      return *this;
    }

    border_style border_style_top() const noexcept {
      return m_border_style_top;
    }

    border_style border_style_bottom() const noexcept {
      return m_border_style_bottom;
    }

    border_style border_style_left() const noexcept {
      return m_border_style_left;
    }

    border_style border_style_right() const noexcept {
      return m_border_style_right;
    }

    border_style border_style_horizontal() const noexcept {
      return m_border_style_horizontal;
    }

    border_style border_style_vertical() const noexcept {
      return m_border_style_vertical;
    }
  private:
    border_style m_border_style_top:4    = border_style::none;
    border_style m_border_style_bottom:4 = border_style::none;
    border_style m_border_style_left:4   = border_style::none;
    border_style m_border_style_right:4  = border_style::none;
    border_style m_border_style_horizontal:4    = border_style::none;
    border_style m_border_style_vertical:4    = border_style::none;
    u8_t m_width = 0U, m_height = 0U;
    const table_cell * m_cells = nullptr;

    friend class table_row;
    friend struct table_rows;
    friend class table_column;
    friend class table_row_iterator;
    friend class table_column_iterator;
    friend struct table_column_border_iterator;
    friend struct table_row_border_iterator;
    friend struct table_columns;
    friend struct table_row_borders;
    friend struct table_column_borders;
  };

  class table_row : private table_column_borders {
  public:
    J_ALWAYS_INLINE table_row(const table_cell * J_NOT_NULL cells, const table * J_NOT_NULL table) noexcept
      : m_cells(cells),
        m_table(table)
    {
      J_ASSERT(m_cells <= m_table->m_cells + m_table->m_width * m_table->m_height, "Table row out of range.");
    }

    J_INLINE_GETTER_NONNULL const table_cell * begin() const noexcept {
      J_ASSERT(m_cells <= m_table->m_cells + m_table->m_width * (m_table->m_height - 1U), "Table row out of range.");
      return m_cells;
    }

    J_INLINE_GETTER_NONNULL const table_cell * end() const noexcept {
      J_ASSERT(m_cells <= m_table->m_cells + m_table->m_width * (m_table->m_height - 1U), "Table row out of range.");
      return m_cells + m_table->columns().size();
    }

    template<typename Fn>
    J_INLINE_GETTER u16_t height(Fn && get_row_height) const noexcept {
      J_ASSERT(m_cells <= m_table->m_cells + m_table->m_width * (m_table->m_height - 1U), "Table row out of range.");
      u16_t height = 0U;
      for (auto & c : *this) {
        height = ::j::max(
          height,
          c.pad_top + c.pad_bottom + static_cast<Fn &&>(get_row_height)(c.content),
          c.pad_height_to);
      }
      return height;
    }

    J_INLINE_GETTER u32_t size() const noexcept { return m_table->m_width; }

    J_INLINE_GETTER const table_cell & operator[](u8_t i) const noexcept {
      J_ASSERT(m_cells <= m_table->m_cells + m_table->m_width * (m_table->m_height - 1U), "Table row out of range.");
      J_ASSERT(i < size(), "Width out of range");
      return m_cells[i];
    }

    const table_column_borders & borders() const noexcept {
      J_ASSERT(m_cells <= m_table->m_cells + m_table->m_width * (m_table->m_height - 1U), "Table row out of range.");
      return *this;
    }

  protected:
    const table_cell * m_cells;
    const table * m_table;
    friend struct table_column_borders;
  };

  class table_column : private table_row_borders {
  public:
    J_ALWAYS_INLINE table_column(const table_cell * J_NOT_NULL cells, const table * J_NOT_NULL table)
      : m_cells(cells),
        m_table(table)
    {
      J_ASSERT_RANGE(m_table->m_cells, m_cells, m_table->m_cells + m_table->m_width + 1U);
    }

    J_INLINE_GETTER u32_t size() const noexcept { return m_table->m_height; }

    column_cell_iterator begin() const noexcept;

    column_cell_iterator end() const noexcept;

    J_INLINE_GETTER const table_cell & operator[](u8_t i) const noexcept {
      J_ASSERT_RANGE(m_table->m_cells, m_cells, m_table->m_cells + m_table->m_width);
      J_ASSERT(i < m_table->m_height, "Table row index out of range");
      return m_cells[m_table->m_width * (u32_t)i];
    }

    template<typename Fn>
    J_INLINE_GETTER u16_t width(Fn && get_row_width) const noexcept {
      J_ASSERT_RANGE(m_table->m_cells, m_cells, m_table->m_cells + m_table->m_width);
      u32_t width = 0U;
      for (u32_t i = 0; i < m_table->rows().size(); ++i) {
        const table_cell & c = operator[](i);
        width = ::j::max(
          width,
          c.padding.pad_left + c.padding.pad_right + ::j::max(c.padding.pad_to, static_cast<Fn &&>(get_row_width)(c.content)));
      }
      return width;
    }

    const table_row_borders & borders() const noexcept {
      J_ASSERT_RANGE(m_table->m_cells, m_cells, m_table->m_cells + m_table->m_width);
      return *this;
    }

  protected:
    const table_cell * m_cells;
    const table * m_table;
    friend struct table_row_borders;
  };

  struct table_column_border_iterator {
    constexpr border_style operator*() noexcept {
      J_ASSERT_RANGE(m_table->m_cells, m_row_start, m_table->m_cells + m_table->m_width * (m_table->m_height - 1) + 1);
      J_ASSERT(m_index <= m_table->m_width, "Iterator out of range.");
      if (m_index == 0U) {
        return combine_border_style(m_row_start[m_index].border_style_left,
                                    border_style::table_default,
                                    m_table->border_style_left());
      } else if (m_index == m_table->m_width) {
        return combine_border_style(m_row_start[m_index - 1U].border_style_right,
                                    border_style::table_default,
                                    m_table->border_style_right());
      } else {
        return combine_border_style(m_row_start[m_index - 1U].border_style_right,
                                    m_row_start[m_index].border_style_left,
                                    m_table->border_style_vertical());
      }
    }

    constexpr table_column_border_iterator & operator++() noexcept {
      J_ASSERT_RANGE(m_table->m_cells, m_row_start, m_table->m_cells + m_table->m_width * (m_table->m_height - 1) + 1);
      J_ASSERT(m_index <= m_table->columns().size(), "Iterator out of range.");
      return ++m_index, *this;
    }

    constexpr table_column_border_iterator operator++(int) noexcept {
      J_ASSERT_RANGE(m_table->m_cells, m_row_start, m_table->m_cells + m_table->m_width * (m_table->m_height - 1) + 1);
      auto result(*this);
      return operator++(), result;
    }

    constexpr bool operator==(const table_column_border_iterator & rhs) const noexcept {
      J_ASSERT_RANGE(m_table->m_cells, m_row_start, m_table->m_cells + m_table->m_width * (m_table->m_height - 1) + 1);
      J_ASSERT(m_row_start == rhs.m_row_start && m_table == rhs.m_table,
               "Invalid iterator comparison");
      return m_index == rhs.m_index;
    }

    constexpr bool operator!=(const table_column_border_iterator & rhs) const noexcept {
      return !operator==(rhs);
    }

    const table_cell * m_row_start;
    const table * m_table;
    u32_t m_index;
  };

  struct table_row_border_iterator {
    constexpr border_style operator*() noexcept {
      J_ASSERT_RANGE(m_table->m_cells, m_row_start, m_table->m_cells + m_table->m_width + 1);
      J_ASSERT(m_index <= m_table->m_height, "Iterator out of range.");
      auto previous = m_index == 0U
        ? m_table->border_style_top()
        : m_row_start[(m_index - 1U) * m_table->m_width].border_style_bottom;
      auto cur = m_index == m_table->m_height
        ? m_table->border_style_bottom()
        : m_row_start[m_index * m_table->m_width].border_style_top;
      return combine_border_style(previous, cur, m_table->border_style_horizontal());
    }

    constexpr table_row_border_iterator & operator++() noexcept {
      J_ASSERT_RANGE(m_table->m_cells, m_row_start, m_table->m_cells + m_table->m_width + 1);
      J_ASSERT(m_index <= m_table->rows().size(), "Iterator out of range.");
      return ++m_index, *this;
    }

    constexpr table_row_border_iterator operator++(int) noexcept {
      J_ASSERT_RANGE(m_table->m_cells, m_row_start, m_table->m_cells + m_table->m_width + 1);
      J_ASSERT(m_index <= m_table->rows().size(), "Iterator out of range.");
      auto result(*this);
      return ++m_index, result;
    }

    constexpr bool operator==(const table_row_border_iterator & rhs) const noexcept {
      J_ASSERT_RANGE(m_table->m_cells, m_row_start, m_table->m_cells + m_table->m_width + 1);
      J_ASSERT(m_row_start == rhs.m_row_start && m_table == rhs.m_table, "Invalid iterator comparison");
      return m_index == rhs.m_index;
    }

    constexpr bool operator!=(const table_row_border_iterator & rhs) const noexcept {
      return !operator==(rhs);
    }

    const table_cell * m_row_start;
    const table * m_table;
    u32_t m_index;
  };

  class table_row_iterator final : private table_row {
  public:
    using table_row::table_row;

    const table_row & operator*() const noexcept {
      J_ASSERT(m_cells < m_table->rows().end().m_cells, "Iterator out of range.");
      return *this;
    }

    const table_row * operator->() const noexcept {
      J_ASSERT(m_cells < m_table->rows().end().m_cells, "Iterator out of range.");
      return this;
    }

    table_row_iterator & operator++() noexcept {
      J_ASSERT(m_cells < m_table->rows().end().m_cells, "Iterator out of range.");
      m_cells += m_table->columns().size();
      return *this;
    }

    table_row_iterator operator++(int) noexcept {
      const auto copy{*this};
      return operator++(), copy;
    }

    bool operator==(const table_row_iterator & rhs) const noexcept {
      J_ASSERT(m_table == rhs.m_table, "Iterators of different tables were compared.");
      return m_cells == rhs.m_cells;
    }

    bool operator!=(const table_row_iterator & rhs) const noexcept {
      return !operator==(rhs);
    }
  };

  class column_cell_iterator {
  public:
    constexpr column_cell_iterator(const table_cell * cell, u8_t width)
      : m_cell(cell),
        m_width(width)
    { J_ASSERT(!m_width == !m_cell, "If width is given, cells must be given."); }

    const table_cell * operator->() const noexcept {
      return m_cell;
    }

    const table_cell & operator*() const noexcept {
      return *m_cell;
    }

    column_cell_iterator & operator++() noexcept {
      m_cell += m_width;
      return *this;
    }

    column_cell_iterator operator++(int) noexcept {
      const auto copy{*this};
      m_cell += m_width;
      return copy;
    }

    bool operator==(const column_cell_iterator & rhs) const noexcept {
      J_ASSERT(m_width == rhs.m_width, "Iterators of different widths were compared.");
      return m_cell == rhs.m_cell;
    }

    bool operator!=(const column_cell_iterator & rhs) const noexcept {
      return !operator==(rhs);
    }
  private:
    const table_cell * m_cell;
    u8_t m_width;
  };

  class table_column_iterator final : private table_column {
  public:
    using table_column::table_column;

    const table_column & operator*() const noexcept {
      J_ASSERT(m_cells < m_table->columns().end().m_cells, "Iterator out of range.");
      return *this;
    }

    const table_column * operator->() const noexcept {
      J_ASSERT(m_cells < m_table->columns().end().m_cells, "Iterator out of range.");
      return this;
    }

    table_column_iterator & operator++() noexcept {
      J_ASSERT(m_cells < m_table->columns().end().m_cells, "Iterator out of range.");
      ++m_cells;
      return *this;
    }

    table_column_iterator operator++(int) noexcept {
      const auto copy{*this};
      return operator++(), copy;
    }

    bool operator==(const table_column_iterator & rhs) const noexcept {
      J_ASSERT(m_table == rhs.m_table, "Iterators of different tables were compared.");
      return m_cells == rhs.m_cells;
    }

    bool operator!=(const table_column_iterator & rhs) const noexcept {
      return !operator==(rhs);
    }
  };

  inline table_row_iterator table_rows::begin() const noexcept {
    auto t = static_cast<const table *>(this);
    return table_row_iterator(t->m_cells, t);
  }

  inline table_row_iterator table_rows::end() const noexcept {
    auto t = static_cast<const table *>(this);
    return table_row_iterator(t->m_cells + t->m_height * t->m_width, t);
  }

  inline table_row table_rows::operator[](u32_t i) const noexcept {
    auto t = static_cast<const table *>(this);
    J_ASSERT(i < t->m_height, "Row index out of range.");
    return table_row(t->m_cells + i * t->m_width, t);
  }

  inline u32_t table_rows::size() const noexcept {
    return static_cast<const table *>(this)->m_height;
  }

  inline table_column_iterator table_columns::begin() const noexcept {
    auto t = static_cast<const table *>(this);
    return table_column_iterator(t->m_cells, t);
  }

  inline table_column_iterator table_columns::end() const noexcept {
    auto t = static_cast<const table *>(this);
    return table_column_iterator(t->m_cells + t->m_width, t);
  }

  inline u32_t table_columns::size() const noexcept {
    return static_cast<const table *>(this)->m_width;
  }

  inline table_row_border_iterator table_row_borders::begin() const noexcept {
    auto t = static_cast<const table_column *>(this);
    return {
      t->m_cells,
      t->m_table,
      0,
    };
  }

  inline table_row_border_iterator table_row_borders::end() const noexcept {
    auto t = static_cast<const table_column *>(this);
    return {
      t->m_cells,
      t->m_table,
      t->size() + 1,
    };
  }

  inline u32_t table_row_borders::size() const noexcept {
    return static_cast<const table_column *>(this)->size() + 1;
  }

  inline table_column_border_iterator table_column_borders::begin() const noexcept {
    auto t = static_cast<const table_row *>(this);
    return {
      t->m_cells,
      t->m_table,
      0U,
    };
  }

  inline table_column_border_iterator table_column_borders::end() const noexcept {
    auto t = static_cast<const table_row *>(this);
    return {
      t->m_cells,
      t->m_table,
      t->size() + 1,
    };
  }

  inline u32_t table_column_borders::size() const noexcept {
    return static_cast<const table_row *>(this)->size() + 1;
  }

  inline table_column table_columns::operator[](u32_t i) const noexcept {
    auto t = static_cast<const table *>(this);
    J_ASSERT(i < t->m_width, "Column index out of range.");
    return table_column(t->m_cells + i, t);
  }
}
