#include "lisp/cir/liveness/live_range.hpp"
#include "lisp/cir/ops/op.hpp"

namespace j::lisp::cir::inline liveness {
  namespace {
    bool can_be_merged_with(const segment & lhs, const segment & rhs) noexcept {
      J_ASSERT(&lhs != &rhs && lhs.index_begin <= lhs.index_end && rhs.index_begin <= rhs.index_end);
      return lhs.index_end >= rhs.index_begin && lhs.index_begin <= rhs.index_end
              && lhs.loc == rhs.loc && (bool)(lhs.loc_specifier.mask & rhs.loc_specifier.mask);
    }

    segment closure(const segment & J_RESTRICT lhs, const segment & J_RESTRICT rhs) noexcept {
      J_ASSERT(lhs.loc == rhs.loc);
      J_ASSUME(&lhs != &rhs);
      J_ASSERT(lhs.loc_specifier.mask & rhs.loc_specifier.mask);
      J_ASSERT(lhs.index_begin <= lhs.index_end && rhs.index_begin <= rhs.index_end);
      return segment{
        .index_begin = min(lhs.index_begin, rhs.index_begin),
        .index_end = max(lhs.index_end, rhs.index_end),
        .loc_specifier = lhs.loc_specifier & rhs.loc_specifier,
        .loc = lhs.loc,
      };
    }

    J_INLINE_GETTER bool operator<(const segment & lhs, const segment & rhs) noexcept {
      J_ASSERT(lhs.index_begin <= lhs.index_end && rhs.index_begin <= rhs.index_end);
      return lhs.index_begin < rhs.index_begin;
    }
  }

  [[nodiscard]] live_range::iterator live_range::segment_at(op_index i) noexcept {
    for (auto & seg : *this) {
      if (i < seg.index_begin) {
        break;
      } else if (i <= seg.index_end) {
        return { &seg, this };
      }
    }
    return live_range::iterator{};
  }

  [[nodiscard]] live_range::const_iterator live_range::segment_at(op_index i) const noexcept
  { return const_cast<live_range*>(this)->segment_at(i); }

  [[nodiscard]] live_range::iterator live_range::segment_at_or_after(op_index i) noexcept {
    for (auto & seg : *this) {
      if (i <= seg.index_end) {
        return { &seg, this };
      }
    }
    return {};
  }

  [[nodiscard]] live_range::const_iterator live_range::segment_at_or_after(op_index i) const noexcept
  { return const_cast<live_range*>(this)->segment_at_or_after(i); }

  [[nodiscard]] bool live_range::contains(op_index i) const noexcept
  { return (bool)segment_at(i); }

  void live_range::add(segment h) {
    J_ASSERT(h.index_begin <= h.index_end);
    J_ASSERT_NOT_NULL(h.loc_specifier);

    if (!size) {
      m_data.val = h;
      size = 1U;
      return;
    }

    if (size == 1U) {
      if (can_be_merged_with(m_data.val, h)) {
        m_data.val = closure(m_data.val, h);
        return;
      }
      segment * data = allocate<segment>(2U);
      const bool is_this_before = m_data.val < h;
      data[is_this_before ? 0 : 1] = m_data.val;
      data[is_this_before ? 1 : 0] = h;
      m_data.ptr = data;
      size = 2;
      return;
    }

    segment data[size + 1U];
    u32_t new_size = 0U;
    bool did_insert = false;
    for (auto & seg : *this) {
      if (!did_insert && can_be_merged_with(seg, h)) {
        h = closure(h, seg);
        continue;
      }
      if (seg.index_begin > h.index_end && !did_insert) {
        data[new_size++] = h;
        did_insert = true;
      }
      data[new_size++] = seg;
    }
    if (!did_insert) {
      if (!new_size) {
        j::free(m_data.ptr);
        m_data.val = h;
        size = 1U;
        return;
      }
      data[new_size++] = h;
    }
    J_ASSUME(new_size > 1U);
    if (size < new_size) {
      m_data.ptr = j::reallocate(m_data.ptr, new_size);
    }
    j::memcpy(m_data.ptr, data, new_size * sizeof(segment));
    size = new_size;
  }

  live_range::iterator live_range::split_at(op_index i, loc to) noexcept {
    J_ASSERT_NOT_NULL(i);
    auto seg = segment_at(i);
    J_ASSERT(seg->index_begin < i && seg->index_end >= i);
    J_ASSERT(seg->index_begin < seg->index_end);
    segment new_seg{
      .index_begin = i,
      .index_end = seg->index_end,
      .loc_specifier = seg->loc_specifier,
      .loc = to,
    };
    seg->index_end = i.prev_half();
    add(new_seg);
    return segment_at(i);
  }

  segment_iterator live_range::move_to_loc_at(op_index i, loc to) noexcept {
    auto seg = segment_at(i);
    J_ASSERT(seg->loc != to);
    if (seg->index_begin == i) {
      seg->loc = to;
      return seg;
    } else {
      return split_at(i, to);
    }
  }

  op * live_range::source_op() const noexcept {
    return J_CONTAINER_OF(this, op, live_range);
  }

  op_index live_range::lower_bound() const noexcept {
    switch (size) {
    case 0: return op_index();
    case 1: return m_data.val.index_begin;
    default: return m_data.ptr[0].index_begin;
    }
  }
}
