#pragma once

#include "ui/layout/flags.hpp"
#include "geometry/vec2.hpp"

namespace j::ui::inline layout {
  struct span final {
    u16_t content_size = 0U;
    u8_t border_before = 0U;
    u8_t margin_before = 0U;
    u8_t border_after = 0U;
    u8_t margin_after = 0U;

    J_INLINE_GETTER constexpr u16_t border_size() const noexcept {
      return content_size == U16_MAX
        ? U16_MAX
        : content_size + border_after + border_after;
    }

    J_INLINE_GETTER constexpr u16_t outer_size() const noexcept {
      return content_size == U16_MAX
        ? U16_MAX
        : content_size
          + border_before + margin_before
          + border_after + margin_after;
    }

    J_INLINE_GETTER constexpr bool operator==(const span & rhs) const noexcept = default;
  };

  inline constexpr span wrap_span(const span & wrapper, span inner, gravity g,
                                  bool collapse_margin_before, bool collapse_margin_after) noexcept {
    i32_t margin_before = inner.margin_before, margin_after = inner.margin_after;
    u16_t inner_size = inner.border_size();
    if (!collapse_margin_before) {
      inner_size += margin_before;
      margin_before = 0;
    }
    if (!collapse_margin_after) {
      inner_size += margin_after;
      margin_after = 0;
    }
    if (wrapper.content_size > inner_size) {
      inner_size = wrapper.content_size;
      switch (g) {
      case gravity::start:
        margin_after -= inner_size - inner.border_size();
        break;
      case gravity::end:
        margin_before -= inner_size - inner.border_size();
        break;
      case gravity::center:
        margin_after -= (inner_size - inner.border_size()) / 2;
        margin_before -= (inner_size - inner.border_size() + 1) / 2;
        break;
      }
    }
    margin_before = ::j::max(margin_before - wrapper.border_before, wrapper.margin_before);
    margin_after = ::j::max(margin_after - wrapper.border_after, wrapper.margin_after);
    return {
      inner_size,
      wrapper.border_before,
      static_cast<u8_t>(margin_before),
      wrapper.border_after,
      static_cast<u8_t>(margin_after),
    };
  }

  inline constexpr span bounding_span(const span & a, const span & b, gravity g) noexcept {
    const u16_t border_size = ::j::max(a.border_size(), b.border_size());
    switch (g) {
    case gravity::start: {
      const u16_t outer_size = ::j::max(a.outer_size(), b.outer_size());
      const u8_t margin_before = ::j::max(a.margin_before, b.margin_before);
      return {
        border_size,
        0,
        margin_before,
        0,
        border_size == U16_MAX
          ? ::j::max(a.margin_after, b.margin_after)
          : static_cast<u8_t>(outer_size - border_size - margin_before),
      };
    }
    case gravity::end: {
      const u16_t outer_size = ::j::max(a.outer_size(), b.outer_size());
      const u8_t margin_after = ::j::max(a.margin_after, b.margin_after);
      return {
        border_size,
        0,
        border_size == U16_MAX
          ? ::j::max(a.margin_before, b.margin_before)
          : static_cast<u8_t>(outer_size - border_size - margin_after),
        0,
        margin_after,
      };
    }
    case gravity::center:
      if (a.border_size() < b.border_size()) {
        const u16_t size_diff = b.border_size() - a.border_size();
        return {
          border_size,
          0,
          static_cast<u8_t>(::j::max(a.margin_before - (size_diff >> 1), b.margin_before)),
          0,
          static_cast<u8_t>(::j::max(a.margin_after - ((size_diff + 1) >> 1), b.margin_after)),
        };
      } else {
        const u16_t size_diff = b.content_size - a.content_size;
        return {
          border_size,
          0,
          static_cast<u8_t>(::j::max(b.margin_before - (size_diff >> 1), a.margin_before)),
          0,
          static_cast<u8_t>(::j::max(b.margin_after - ((size_diff + 1) >> 1), a.margin_after)),
        };
      }
    }
  }

  inline constexpr span concatenate_span(const span & a, const span & b) noexcept {
    u8_t margin_between = ::j::max(a.margin_after, b.margin_before);
    u16_t content_size = a.content_size + b.content_size + margin_between;
    if (margin_between) {
      content_size += a.border_after + b.border_before;
    } else {
      content_size += ::j::max(a.border_after, b.border_before);
    }
    return {
      content_size,
      a.border_before,
      a.margin_before,
      b.border_after,
      b.margin_after,
    };
  }

  struct size final {
    span horizontal, vertical;

    constexpr span & operator[](geometry::axis a) noexcept {
      return a == geometry::axis::vertical ? vertical : horizontal;
    }

    constexpr const span & operator[](geometry::axis a) const noexcept {
      return a == geometry::axis::vertical ? vertical : horizontal;
    }

    constexpr geometry::vec2u16 content_size() const noexcept {
      return { horizontal.content_size, vertical.content_size };
    }

    constexpr geometry::vec2u16 border_size() const noexcept {
      return { horizontal.border_size(), vertical.border_size() };
    }

    constexpr geometry::vec2u16 outer_size() const noexcept {
      return { horizontal.outer_size(), vertical.outer_size() };
    }

    constexpr bool operator==(const size & rhs) const noexcept {
      return vertical == rhs.vertical && horizontal == rhs.horizontal;
    }
  };

  inline constexpr size bounding_size(const size & a, const size & b, orientation o, gravity cross_gravity) noexcept {
    size result;
    result[main_axis(o)] = concatenate_span(a[main_axis(o)], b[main_axis(o)]);
    result[cross_axis(o)] = bounding_span(a[cross_axis(o)], b[cross_axis(o)], cross_gravity);
    return result;
  }

  inline constexpr size wrapped_size(const size & content_size, const size & min_wrapper_size, gravity h_gravity, gravity v_gravity) {
    return {
      bounding_span(content_size.horizontal, min_wrapper_size.horizontal, h_gravity),
      bounding_span(content_size.vertical, min_wrapper_size.vertical, v_gravity),
    };
  }
}
