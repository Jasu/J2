#include "tty/components/line_table.hpp"
#include "tty/termpaint_style_map.hpp"
#include "logging/global.hpp"
#include "strings/unicode/rope/rope_utf8_hard_lbr_iterator.hpp"
#include "strings/unicode/rope/rope_debug.hpp"

namespace j::tty::inline components {
  namespace s = strings;
  namespace {
    [[maybe_unused]] void dump_hard_lines(const vector<line_table::hard_line> & lns) {
      u32_t i = 0;
      for (auto & ln : lns) {
        J_DEBUG("#{}: {}", i++, ln.end.as_byte_iterator().position());
        u32_t j = 0;
        for (auto sln : ln.lines) {
          J_DEBUG("  #{}: {}b {} cols", j++, sln.bytes, sln.width);
        }
      }
    }
  }

  line_table::line_table(i16_t prefix_width) noexcept
    : prefix_width(prefix_width)
  {
    text_style = prefix_style = style_map->get_attr(strings::styling::g_empty_style);
  }

  void line_table::compute_all_soft_lines(output_context & ctx, i16_t width) {
    total_lines = 0;
    width -= prefix_width;
    auto prev = value.bytes().begin();
    for (auto it = hard_lines.begin(), end = hard_lines.end(); it != end; ++it) {
      auto text_it = it->end.as_byte_iterator();
      it->lines.resize(0);
      s::const_rope_utf8_view line{prev, it == end - 1 ? text_it : text_it - 1};
      prev = text_it;

      do {
        auto m = ctx.measure(line, { .columns = width }, true);
        it->lines.push_back({
            m.columns,
            m.bytes,
          });
        ++total_lines;
        if (!m.bytes || (u64_t)m.bytes == line.size()) {
          break;
        }
        line.remove_prefix(m.bytes);
      } while (line);
    }
  }

  void line_table::compute_hard_lines(output_context & ctx) {
    max_line_width = 0;
    u32_t num_hard_lines = value.num_hard_breaks() + 1U;
    if (!value.empty() && *(value.bytes().end() - 1) == '\n') {
      num_hard_lines++;
    }
    for (auto & hl: hard_lines) {
      hl.end.invalidate();
    }
    hard_lines.resize(num_hard_lines);
    auto cur_line = hard_lines.begin();

    auto it = s::rope_utf8_hard_lbr_iterator(value.bytes().begin()),
         end = s::rope_utf8_hard_lbr_iterator(value.bytes().end());
    auto begin = it;
    if (it != end) {
      ++it;
    }
    for (; it != end; ++it) {
      s::const_rope_utf8_view line{*begin, *it};
      cur_line->width = ctx.measure(line).columns;
      max_line_width = max(max_line_width, cur_line->width);
      (cur_line++)->end = s::marker{*it};
      begin = it;
    }
    if (!value.empty() && *(value.bytes().end() - 1) == '\n') {
      cur_line->width = 0;
      (cur_line++)->end = s::marker{value.bytes().end()};
    }
    cur_line->width = 0;
    (cur_line)->end = s::marker{value.bytes().end()};
  }

  strings::rope_utf8_byte_iterator line_table::line_begin(strings::rope_utf8_byte_iterator it) noexcept {
    strings::rope_utf8_byte_iterator res = value.bytes().begin();
    strings::rope_utf8_byte_iterator end = value.bytes().begin();
    for (auto & hl : hard_lines) {
      res = end;
      end = hl.end.as_byte_iterator();
      if (it.position() < end.position()) {
        return res;
      }
    }
    return res;
  }

  strings::rope_utf8_byte_iterator line_table::line_end(strings::rope_utf8_byte_iterator it) noexcept {
    for (auto & hl : hard_lines) {
      if (it.position() < hl.end.as_byte_iterator().position()) {
        it = hl.end.as_byte_iterator() - 1;
        if (*it != '\n') {
          ++it;
        }
        return it;
      }
    }
    return value.bytes().end();
  }

  strings::rope_utf8_byte_iterator line_table::visual_line_begin(strings::rope_utf8_byte_iterator it) noexcept {
    auto pos = it.position();
    for (auto & hl : hard_lines) {
      for (auto l : hl.lines) {
        if (pos <= (u64_t)l.bytes) {
          return it - pos;
        }
        pos -= l.bytes;
      }
      --pos;
    }
    J_FAIL("Line begin at end");
  }

  void line_table::set_text(strings::const_string_view str) {
    for (auto & hl: hard_lines) {
      hl.end.invalidate();
    }
    hard_lines.clear();
    value = str;
  }

  strings::rope_utf8_byte_iterator line_table::visual_line_end(strings::rope_utf8_byte_iterator it) noexcept {
    auto pos = it.position();
    for (auto & hl : hard_lines) {
      for (auto l : hl.lines) {
        if (pos < (u64_t)l.bytes) {
          return it + (l.bytes - pos);
        }
        pos -= l.bytes;
      }
      --pos;
    }
    return value.bytes().end();
  }

  void line_table::set_text_style(const strings::style & style) noexcept {
    text_style = style_map->get_attr(style);
  }
  void line_table::set_initial_prefix_style(const strings::style & style) noexcept {
    initial_prefix_style = style_map->get_attr(style);
  }
  void line_table::set_prefix_style(const strings::style & style) noexcept {
    prefix_style = style_map->get_attr(style);
  }
  void line_table::render(output_context & ctx) {
    ctx.clear();

    auto prev = value.bytes().begin();
    i32_t y = 0;
    strings::const_string_view cur_prefix = initial_prefix;
    attr cur_prefix_style = initial_prefix_style ? initial_prefix_style : prefix_style;
    for (auto it = hard_lines.begin(), end = hard_lines.end(); it != end; ++it) {
      auto text_it = it->end.as_byte_iterator();
      s::const_rope_utf8_view line{prev, it == end - 1 ? text_it : text_it - 1};
      prev = text_it;
      for (auto l : it->lines) {
        if (y >= 0) {
          ctx.write(0, y, cur_prefix, cur_prefix_style);
          auto cur_ln = ((u64_t)l.bytes == line.size()) ? line : line.take_prefix(l.bytes);
          ctx.write(prefix_width, y, cur_ln, text_style);
          cur_prefix = prefix;
          cur_prefix_style = prefix_style;
        }
        ++y;
      }
    }
  }

  pos line_table::get_pos(output_context & ctx, strings::const_rope_utf8_byte_iterator it) const noexcept {
    pos result{(i16_t)prefix_width, 0};
    if (it == value.bytes().begin()) {
      return result;
    }
    strings::const_rope_utf8_byte_iterator line_begin = value.bytes().begin();
    for (auto & hl : hard_lines) {
      for (auto l : hl.lines) {
        strings::const_rope_utf8_byte_iterator line_end = line_begin + l.bytes;
        if (it.position() <= line_end.position()) {
          s::const_rope_utf8_view line{line_begin, it};
          result.x += ctx.measure(line).columns;
          if (result.x == ctx.width()) {
            result.x = prefix_width;
            ++result.y;
          }
          return result;
        }
        line_begin = line_end;
        result.y++;
      }
      ++line_begin;
    }
    J_DEBUG("Pos out of range", it.position(), value.size_bytes());
    J_FAIL("");
  }

  cursor_offsets line_table::offsets(output_context & ctx, pos p) {
    strings::const_rope_utf8_byte_iterator begin = value.bytes().begin();
    strings::const_rope_utf8_byte_iterator end = value.bytes().end();
    p.x -= prefix_width;
    for (auto & hl : hard_lines) {
      for (auto l : hl.lines) {
        end = begin + l.bytes;
        if (!p.y) {
          break;
        }
        --p.y;
        begin = end;
      }
      if (!p.y) {
        break;
      }
      ++begin;
    }
    s::const_rope_utf8_view line{begin, end};
    return ctx.offsets(p.x, line);
  }

  strings::rope_utf8_byte_iterator line_table::cursor_at(output_context & ctx, pos p) noexcept {
    strings::rope_utf8_byte_iterator begin = value.bytes().begin();
    strings::rope_utf8_byte_iterator end = value.bytes().end();
    p.x -= prefix_width;
    bool done = false;
    for (auto & hl : hard_lines) {
      for (auto l : hl.lines) {
        end = begin + l.bytes;
        if (!p.y) {
          done = true;
          break;
        }
        --p.y;
        begin = end;
      }
      if (done) {
        break;
      }
      ++begin;
    }
    s::const_rope_utf8_view line{begin, end};
    return begin + ctx.measure(line, { .columns = p.x }).bytes;
  }
}
