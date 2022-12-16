#include <detail/preamble.hpp>

#include "strings/unicode/utf8_rope.hpp"
#include "strings/unicode/utf8_rope_debug.hpp"
#include "strings/unicode/utf8_rope_lbr_iterator.hpp"
#include "rope.hpp"

namespace s = j::strings;

struct br final {
  const char * contents = nullptr;
  s::line_break_type type = s::line_break_type::opportunity;

  constexpr br() noexcept = default;

  explicit constexpr br(const char * contents) noexcept
    : contents(contents)
  { }

  constexpr br(const char * contents, s::line_break_type t) noexcept
    : contents(contents),
      type(t)
  { }

  constexpr br operator!() const noexcept {
    return br(contents, s::line_break_type::mandatory);
  }
};

inline void assert_line_break(const s::utf8_rope_view & v, s::const_string_view expected) {
  char cluster[v.size()];
  v.copy_to(cluster, v.size());
  s::const_string_view cluster_view(cluster, v.size());
  REQUIRE(cluster_view == expected);
}

template<u32_t N>
void assert_line_breaks(const s::utf8_rope & r, const br (&breaks)[N]) {
  auto it = r.line_breaks().begin();
  s::utf8_rope_code_point_iterator previous = r.begin();
  const auto end = r.line_breaks().end();
  u32_t num_mandatory = 0U;
  try {
    for (u32_t i = 0U, pos = 0U; i < N; ++i) {
      assert_line_break(s::utf8_rope_view(previous, *it), breaks[i].contents);
      pos += ::j::strlen(breaks[i].contents);
      if (it.line_break_type() == s::line_break_type::mandatory) {
        CAPTURE(pos);
        if (pos) {
          REQUIRE(r.num_lines_before(pos - 1) == num_mandatory);
        }
        ++num_mandatory;
        REQUIRE(r.line_begin_index(num_mandatory) == pos);
        REQUIRE(r.num_lines_before(pos) == num_mandatory);
      }
      if (breaks[i].type != s::line_break_type::none) {
        REQUIRE(it.line_break_type() == breaks[i].type);
      }
      if (i != N - 1) {
        previous = *it++;
      }
    }
    REQUIRE(it == end);
    REQUIRE(r.line_begin_index(0U) == 0);
    REQUIRE(r.num_lines() == num_mandatory);
  } catch (...) {
    J_DUMP_UTF8_ROPE(r);
    throw;
  }
}

inline void assert_line_breaks(const s::utf8_rope & r, u32_t n, const char * const * const breaks) {
  auto it = r.line_breaks().begin();
  const auto end = r.line_breaks().end();
  s::utf8_rope_code_point_iterator previous = r.begin();
  for (u32_t i = 0U; i < n; ++i) {
    assert_line_break(s::utf8_rope_view(previous, *it), breaks[i]);
    if (i != n - 1) {
      previous = *it++;
    }
  }
  REQUIRE(it == end);
}
