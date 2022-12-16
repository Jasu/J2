#include "table.hpp"
#include "strings/format.hpp"
#include "hzd/math.hpp"
#include "logging/global.hpp"

#include "containers/trivial_array.hpp"

J_DEFINE_EXTERN_TRIVIAL_ARRAY(j::graphviz::table_column);

namespace j::graphviz {
  namespace {
    namespace s = strings;
    table_style merge_styles(const table_style & base, const table_style & overrides) {
      table_style res = base;
      if (overrides.bg_color) {
        res.bg_color = overrides.bg_color;
      }
      if (overrides.fg_color) {
        res.fg_color = overrides.fg_color;
      }
      if (overrides.font_size) {
        res.font_size = overrides.font_size;
      }
      if (overrides.align) {
        res.align = overrides.align;
      }
      if (overrides.valign) {
        res.valign = overrides.valign;
      }
      if (overrides.bold) {
        res.bold = overrides.bold;
      }
      if (overrides.italic) {
        res.italic = overrides.italic;
      }
      if (overrides.padding >= 0) {
        res.padding = overrides.padding;
      }
      return res;
    }

    constexpr inline const char * const aligns[] = {
      [align_none]   = "",
      [align_right]  = " align=\"right\"",
      [align_left]   = " align=\"left\"",
      [align_center] = " align=\"center\"",
    };

    constexpr inline const char * const valigns[] = {
      [valign_none]   = "",
      [valign_top]    = " valign=\"top\"",
      [valign_bottom] = " valign=\"bottom\"",
      [valign_middle] = " valign=\"middle\"",
    };

    [[nodiscard]] inline s::string format_point_size(float point_size) {
      return s::format(
        ceil(point_size) == point_size ? "{}.0" : "{}",
        point_size);
    }
  }

  table_row::table_row(u32_t sz, const table_style & style)
    : cols(sz),
      style(style)
  {
    for (u32_t i = 0; i < sz; ++i) {
      cols[i].style = style;
    }
  }

  table_row::table_row(const table_style & style, table_column * J_NOT_NULL begin, u32_t sz)
    : cols(containers::move, begin, sz),
      style(style)
  {
    for (u32_t i = 0; i < sz; ++i) {
      cols[i].style = merge_styles(style, cols[i].style);
    }
  }

  table_row::table_row(const table_style & style, s::string * J_NOT_NULL begin, u32_t sz)
    : cols(sz),
      style(style)
  {
    for (u32_t i = 0; i < sz; ++i) {
      cols[i].style = style;
      cols[i].value = static_cast<strings::string &&>(begin[i]);
    }
  }
  table::table(const table_style & st) noexcept {
    set_style(st);
  }

  void table::set_style(const table_style & st) noexcept {
    style = st;
    if (!style.align) {
      style.align = align_left;
    }
    if (!style.valign) {
      style.valign = valign_middle;
    }
    if (style.padding < 0) {
      style.padding = 3;
    }
  }

  [[nodiscard]] table_row & table::push_row(u32_t width,  const table_style & style) {
    return rows.emplace_back(width, merge_styles(this->style, style));
  }

  [[nodiscard]] table_row & table::push_row(u32_t width) {
    return rows.emplace_back(width, style);
  }

  table_row & table::push_row(table_column * J_NOT_NULL begin, u32_t sz) {
    return rows.emplace_back(style, begin, sz);
  }

  table_row & table::push_row(const table_style & style, strings::string * J_NOT_NULL begin, u32_t sz) {
    return rows.emplace_back(merge_styles(this->style, style), begin, sz);
  }

  void table::clear() noexcept {
    rows.clear();
  }

  strings::string table::render() const noexcept {
    u32_t max_span = 0U;
    for (auto & row : rows) {
      max_span = max(max_span, row.is_flex ? 1U : row.size());
    }

    const strings::const_string_view table_fg = style.fg_color.format();
    const strings::const_string_view table_bg = style.bg_color.format();
    const i8_t table_padding = style.padding;
    const valign table_valign = style.valign ? valign_middle : valign_none;

    strings::string result =
      strings::format("<<font point-size=\"{}\" color=\"{}\"><table cellspacing=\"0\" cellborder=\"0\" cellpadding=\"{}\" border=\"2\""
                      " bgcolor=\"{}\" color=\"{}\"{}{}>\n",
                      format_point_size(style.font_size),
                      table_fg,
                      table_padding,
                      table_bg, table_bg,
                      valigns[table_valign == valign_middle ? valign_none : table_valign],
                      port ? " port=\"" + port + "\"" : "");

    color cur_color{};
    bool is_first_row = true;
    for (auto & row : rows) {
      bool did_differ = false;
      bool all_table_bg = true;
      strings::string row_str;
      u8_t col_index = 0U;
      bool had_mixed = false;

      u32_t span_err = max_span;
      if (row.is_flex) {
        row_str = "<td cellpadding=\"0\"";
        row_str += aligns[style.align];
        if (span_err != 1) {
          row_str += strings::format(" colspan=\"{}\"", span_err);
        }
        row_str += strings::format("><table cellspacing=\"0\" border=\"0\" cellpadding=\"3\"><tr>");
        span_err = 0U;
      }
      u32_t span_per_col = span_err / row.size();
      span_err -= span_per_col * row.size();
      for (auto & col : row.cols) {
        strings::const_string_view col_bg = col.style.bg_color.format();
        all_table_bg &= col.style.bg_color == table_bg;
        if (col.style.bg_color != cur_color) {
          if (col_index) {
            if (cur_color != table_bg && col_bg != table_bg) {
              row_str += "<vr/>";
            }
            had_mixed = true;
          }
          did_differ = true;
          cur_color = col_bg;
        }
        row_str += "<td";
        if (col.port) {
          row_str += " port=\"" + col.port + "\"";
        }
        if (col.href || col.tooltip) {
          row_str += " href=\"" + (col.href ? col.href : "#") + "\"";
        }
        if (col.tooltip) {
          row_str += " title=\"" + col.tooltip + "\"";
        }
        if (col.id) {
          row_str += " id=\"" + col.id + "\"";
        }
        if (col.style.padding >= 0 && col.style.padding != table_padding) {
          row_str += s::format(" cellpadding=\"{}\"", col.style.padding);
        }
        u32_t col_span = span_per_col;
        if (span_err) {
          ++col_span;
          --span_err;
        }
        if (col_span > 1) {
          row_str += strings::format(" colspan=\"{}\"", col_span);
        }

        if (col_bg != table_bg) {
          row_str += " bgcolor=\"" + col_bg + "\"";
        }
        if (col.style.valign != valign_none && col.style.valign != table_valign) {
          row_str += valigns[col.style.valign];
        }
        row_str += aligns[col.style.align];
        row_str.push_back('>');

        strings::const_string_view col_fg = col.style.fg_color.format();

        if (!col.value) {
          row_str += "</td>";
        } else {
          strings::string suffix = "</td>";
          if (col_fg != table_fg || col.style.font_size != style.font_size) {
            row_str+= "<font";
            if (col_fg != "#000000") {
              row_str += " color=\"" + col_fg + "\"";
            }
            if (col.style.font_size != style.font_size) {
              row_str += " point-size=\"" + format_point_size(col.style.font_size) + "\"";
            }
            row_str.push_back('>');
            suffix ="</font></td>";
          }
          if (col.style.bold) {
            row_str += "<b>";
            suffix = "</b>" + suffix;
          }
          if (col.style.italic != style.italic) {
            row_str += "<i>";
            suffix = "</i>" + suffix;
          }
          row_str += col.value + suffix;
        }
        ++col_index;
      }
      if (row.is_flex) {
        row_str += "</tr></table></td>";
      }
      result += ((did_differ && !all_table_bg && !is_first_row) ? "<hr/><tr>" : "<tr>") + row_str + "</tr>";
      is_first_row = all_table_bg;
      if (had_mixed) {
        cur_color.reset();
      }
    }
    return result + "</table></font>>";
  }
}
