#include "rendering/texture_atlas/row_allocator.hpp"

#include "exceptions/assert_lite.hpp"
#include "containers/vector.hpp"
#include "logging/global.hpp"

J_DEFINE_EXTERN_VECTOR(j::rendering::texture_atlas::span);
J_DEFINE_EXTERN_VECTOR(j::rendering::texture_atlas::row);

namespace j::rendering::texture_atlas {
  row_allocator::row_allocator(u16_t width, u16_t height)
    : width(width),
      height(height)
  {
    J_ASSUME(width && height);
  }

  row * row_allocator::get_row(geometry::vec2u16 size, u16_t & top) {
    u16_t match_size = U16_MAX;
    row * result = nullptr;
    u16_t cur_top = 0;

    for (auto & row : m_rows) {
      if (row.max_free_width >= size.x && row.height >= size.y && row.height < match_size) {
        top = cur_top, result = &row;
        if (row.height == size.y) {
          return result;
        }
        match_size = row.height;
      }
      cur_top += row.height;
    }
    if (result) {
      return result;
    }

    if (!m_rows.empty() && m_rows.back().max_free_width >= size.x) {
      const u16_t height_diff = size.y - m_rows.back().height;
      if ((height_diff <= 4U || height < size.y) && J_LIKELY(height_diff <= height)) {
        top = cur_top - m_rows.back().height;
        height -= height_diff;
        m_rows.back().height = size.y;
        return &m_rows.back();
      }
    }

    if (J_UNLIKELY(height < size.y)) {
      return nullptr;
    }
    height -= size.y, top = cur_top;
    return &m_rows.emplace_back(row{
      .free = vector<span>((span[]){{0, width}}),
      .height = size.y,
      .max_free_width = width,
    });
  }

  geometry::rect_u16 row_allocator::allocate(geometry::vec2u16 size) {
    J_ASSERT_NOT_NULL(size.x, size.y);

    u16_t top;
    auto row = get_row(size, top);
    if (J_UNLIKELY(!row)) {
      return geometry::rect_u16{0, 0, 0, 0};
    }

    span * best_span = nullptr;
    u16_t best_width = U16_MAX;
    for (auto & span : row->free) {
      u16_t width = span.right - span.left;
      J_ASSERT_NOT_NULL(width);
      if (width >= size.x && width < best_width) {
        best_span = &span;
        best_width = width;
      }
    }

    J_ASSERT(best_span, "Could not find a free region in a row.");

    geometry::rect_u16 result{
      best_span->left,
      top,
      size,
    };
    if (J_UNLIKELY(best_width == size.x)) {
      row->free.erase(best_span);
    } else if (J_UNLIKELY(best_span->right == width)) {
      best_span->right -= size.x;
      result.set_left(best_span->right);
    } else {
      best_span->left += size.x;
    }
    if (best_width == row->max_free_width) {
      row->max_free_width = 0U;
      for (auto & span : row->free) {
        row->max_free_width = j::max(row->max_free_width, span.right - span.left);
      }
    }
    return result;
  }

  void row_allocator::erase(geometry::rect_u16 rect) {
    J_ASSERT_NOT_NULL(rect.width(), rect.height());

    // Find the row to erase from.
    auto row_it = m_rows.begin();
    for (u16_t top = 0; top != rect.top(); top += row_it->height, ++row_it) { }

    u16_t left = rect.left(), right = rect.right();
    const auto end = row_it->free.end();
    auto it = row_it->free.begin(), prev = it;
    for (; it != end && it->right <= left; ++it) { prev = it; }
    if (prev != end && prev->right == left) {
      // Inserting directly after a free span - expand the span.
      if (J_UNLIKELY(it != end && it->left == right)) {
        // Also inserting directly before - merge the spans.
        right = it->right;
        row_it->free.erase(it);
      }
      prev->right = right, left = prev->left;
    } else if (it != end && it->left == right) {
      // Inserting directly before a free span - expand the next span.
      it->left = left, right = it->right;
    } else {
      // Inserting somewhere in the middle, or as the last or only element.
      row_it->free.insert(it, { left, right });
    }
    row_it->max_free_width = j::max(row_it->max_free_width, right - left);
    J_ASSERT(row_it->max_free_width <= width, "Width overflowed.");

    // If the last row was completely emptied. Erase the row, and any preceding
    // empty rows (the current allocator does not allow for removing / merging nodes
    // in the middle, so empty rows may be left in the middle).
    if (J_UNLIKELY(row_it->max_free_width == width) && row_it + 1 == m_rows.end()) {
      do {
        height += (row_it--)->height;
        m_rows.pop_back();
      } while (J_UNLIKELY(!m_rows.empty() && row_it->max_free_width == width));
    }
  }

  void row_allocator::dump() const {
    J_DEBUG("{#bright_yellow_bg,black,bold}  Texture atlas dump  {/}");
    u32_t i = 0U, y = 0U;
    for (auto & row : m_rows) {
      J_DEBUG("   {#bright_green,bold}Row #{}:{/} Y={#bright_cyan,bold}{}{/} H={#bright_magenta,bold}{}{/} Max free={#bright_green,bold}{}{/}",
              i, y, row.height, row.max_free_width);
      u32_t j = 0U;
      for (auto & s : row.free) {
        J_DEBUG("            {#bright_cyan}Span #{}:{/} {#bright_yellow}({}, {}){/} W={#bright_green}{}{/}",
                j, s.left, s.right, s.right - s.left);
        ++j;
      }
      y += row.height;
      ++i;
    }
    J_DEBUG("\n");
  }
}
