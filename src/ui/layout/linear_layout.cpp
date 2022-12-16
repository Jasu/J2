#include "ui/layout/linear_layout.hpp"

#include "containers/vector.hpp"

J_DEFINE_EXTERN_NONCOPYABLE_VECTOR(j::mem::shared_ptr<j::ui::element>);

namespace j::ui {
  inline namespace layout {
    bool linear_layout::collapse_main_before() const noexcept {
      return m_flags.has(orientation() == orientation::columns
                         ? layout_flags_t::collapse_margin_top
                         : layout_flags_t::collapse_margin_left);
    }

    bool linear_layout::collapse_main_after() const noexcept {
      return m_flags.has(orientation() == orientation::columns
                         ? layout_flags_t::collapse_margin_bottom
                         : layout_flags_t::collapse_margin_right);
    }

    bool linear_layout::collapse_cross_before() const noexcept {
      return m_flags.has(orientation() == orientation::columns
                         ? layout_flags_t::collapse_margin_left
                         : layout_flags_t::collapse_margin_top);
    }

    bool linear_layout::collapse_cross_after() const noexcept {
      return m_flags.has(orientation() == orientation::columns
                         ? layout_flags_t::collapse_margin_right
                         : layout_flags_t::collapse_margin_bottom);
    }
    void linear_layout::push_back(mem::shared_ptr<element> e) {
      J_ASSERT_NOT_NULL(e);
      m_elements.push_back(static_cast<mem::shared_ptr<element> &&>(e));
    }

    size linear_layout::min_size() const noexcept {
      span main, cross = m_box.min_size()[cross_axis()];
      bool is_first = true;
      for (auto & b : m_elements) {
        cross = wrap_span(cross, b->min_size()[cross_axis()], cross_gravity(), collapse_cross_before(), collapse_cross_after());
        if (!is_first) {
          main = concatenate_span(main, b->min_size()[main_axis()]);
        } else {
          main = b->min_size()[main_axis()];
          is_first = false;
        }
      }

      main = wrap_span(m_box.min_size()[main_axis()], main, main_gravity(), collapse_main_before(), collapse_main_after());
      return orientation() == orientation::columns ? size{ main, cross } : size{ cross, main };
    }

    g::vec2u16 linear_layout::max_content_size() const noexcept {
      return m_box.max_content_size;
    }

    void linear_layout::render(context & ctx, const geometry::rect_u16 & region) const {
      m_frame.render(ctx, region, m_box.border, m_style);
      if (m_elements.empty()) {
        return;
      }
      u16_t sizes[m_elements.size()];
      u16_t margins[m_elements.size() - 1U];
      ::j::memzero(sizes, sizeof(sizes));
      ::j::memzero(margins, sizeof(margins));
      const bool is_cols = orientation() == orientation::columns;
      // const bool collapse_main_before = m_flags.has(
      //   is_cols ? layout_flags_t::collapse_margin_left : layout_flags_t::collapse_margin_top);
      // const bool collapse_main_after = m_flags.has(
      //   is_cols ? layout_flags_t::collapse_margin_right : layout_flags_t::collapse_margin_bottom);
      // if (collapse_main_before) {
      //   region.width() -= m_elements[0].min_size().
      // }

      u16_t size_left = is_cols ? region.width() : region.height();
      u32_t sink_capacity = 0U;
      u32_t num_sinks = 0U;
      size_left -= is_cols
        ? m_box.margin.left + m_box.margin.right
        : m_box.margin.top + m_box.margin.bottom;

      u32_t i = 0;
      for (auto & b : m_elements) {
        auto min_size = b->min_size()[main_axis()];
        sizes[i] = min_size.border_size();
        size_left -= sizes[i];
        const u32_t max_sz = is_cols ? b->max_content_size().x : b->max_content_size().y;
        if (max_sz > sizes[i]) {
          sink_capacity += max_sz - sizes[i];
          ++num_sinks;
        }
        if (i != m_elements.size() - 1U) {
          margins[i] = min_size.margin_after;
        }
        if (i != 0) {
          margins[i - 1U] = ::j::max(margins[i - 1U], min_size.margin_before);
          size_left -= margins[i - 1U];
        }
        ++i;
      }

      while (size_left && sink_capacity) {
        const u32_t size = size_left / num_sinks;
        u32_t error = size_left - size * num_sinks;
        i = 0;
        for (auto & b : m_elements) {
          const i32_t sink = (is_cols ? b->max_content_size().x : b->max_content_size().y) - sizes[i];
          if (sink > 0) {
            i32_t sinked = ::j::min(size, sink);
            if (sinked < sink && error) {
              --error, ++sinked;
            }
            if (sinked == sink) {
              --num_sinks;
            }
            sizes[i] += sinked;
            size_left -= sinked;
            sink_capacity -= sinked;
          }
          i++;
        }
      }

      const u32_t size = size_left / (m_elements.size() - 1U);
      u32_t error = size * (m_elements.size() - 1U);
      for (u32_t i = 0; i < m_elements.size() - 1U; ++i) {
        margins[i] += size;
        if (error) {
          ++margins[i], --error;
        }
      }

      i = 0;
      u16_t pos = 0;//is_cols ? region.left() : region.top();
      for (auto & e : m_elements) {
        if (i != 0) {
          pos += margins[i - 1];
        }
        const g::rect_u16 subregion(
          (is_cols ? pos : 0) + m_box.border.left,
          (is_cols ? 0 : pos) + m_box.border.top,
          is_cols ? sizes[i] : region.width() - m_box.border.width(),
          is_cols ? region.height() : sizes[i] - m_box.border.height());
        e->render(ctx, subregion);
        pos += sizes[i];
        ++i;
      }
    }
  }
}
