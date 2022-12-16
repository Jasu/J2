#pragma once

#include "lisp/cir/liveness/live_range_fwd.hpp"

namespace j::lisp::cir::inline liveness {
  template<typename Seg, typename Lr>
  struct basic_segment_iterator final {
    Seg * seg = nullptr;
    Lr * rng = nullptr;

    J_A(AI) basic_segment_iterator & operator++() noexcept {
      ++seg;
      return *this;
    }

    J_A(AI) basic_segment_iterator operator++(int) noexcept {
      return { ++seg, rng };
    }

    J_A(AI) basic_segment_iterator & operator-=(i32_t index) noexcept {
      seg -= index;
      return *this;
    }

    J_A(AI) basic_segment_iterator & operator+=(i32_t index) noexcept {
      seg += index;
      return *this;
    }

    J_A(AI,NODISC) i32_t operator-(const basic_segment_iterator & rhs) const noexcept {
      return seg - rhs.seg;
    }

    J_A(AI,NODISC) basic_segment_iterator operator+(i32_t index) const noexcept {
      return { seg + index, rng };
    }

    J_A(AI,NODISC) basic_segment_iterator operator-(i32_t index) const noexcept {
      return { seg - index, rng };
    }

    J_INLINE_GETTER constexpr bool operator==(const basic_segment_iterator &) const noexcept = default;

    J_INLINE_GETTER constexpr bool operator<(const basic_segment_iterator & rhs) const noexcept {
      return seg < rhs.seg;
    }

    J_INLINE_GETTER constexpr bool operator==(op_index index) const noexcept {
      return seg->begin_index == index;
    }

    J_INLINE_GETTER constexpr bool operator<(op_index index) const noexcept {
      return seg->begin_index < index;
    }

    J_INLINE_GETTER explicit operator bool() const noexcept {
      return seg;
    }

    J_INLINE_GETTER bool operator!() const noexcept {
      return !seg;
    }

    J_A(AI,RNN,NODISC) Seg * operator->() const noexcept {
      return seg;
    }

    J_A(AI,NODISC) Seg & operator*() const noexcept {
      return *seg;
    }

    J_A(AI,NODISC) operator basic_segment_iterator<const Seg, const Lr>() const noexcept {
      return {seg, rng};
    }
  };

  using segment_iterator = basic_segment_iterator<segment, live_range>;
  using const_segment_iterator = basic_segment_iterator<const segment, const live_range>;
}
