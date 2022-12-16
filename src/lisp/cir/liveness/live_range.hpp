#pragma once

#include "lisp/cir/liveness/live_range_fwd.hpp"
#include "lisp/cir/liveness/segment_iterator.hpp"

namespace j::lisp::cir::inline liveness {
  [[nodiscard]] inline live_range::iterator live_range::begin() noexcept
  { return {size != 1U ? m_data.ptr : &m_data.val, this}; }
  [[nodiscard]] inline live_range::const_iterator live_range::begin() const noexcept
  { return {size != 1U ? m_data.ptr : &m_data.val, this}; }
  [[nodiscard]] inline live_range::iterator live_range::end() noexcept
  { return {size != 1U ? m_data.ptr + size : (&m_data.val + 1), this}; }
  [[nodiscard]] inline live_range::const_iterator live_range::end() const noexcept
  { return {size != 1U ? m_data.ptr + size : (&m_data.val + 1), this}; }

  [[nodiscard]] inline live_range::iterator live_range::front() noexcept {
    J_ASSERT(size);
    return begin();
  }

  [[nodiscard]] inline live_range::const_iterator live_range::front() const noexcept {
    J_ASSERT(size);
    return begin();
  }

  [[nodiscard]] inline live_range::iterator live_range::back() noexcept {
    J_ASSERT(size);
    return live_range::iterator{(size != 1U ? m_data.ptr + size - 1 : &m_data.val), this};
  }

  [[nodiscard]] inline live_range::const_iterator live_range::back() const noexcept {
    J_ASSUME(size != 0U);
    return live_range::const_iterator{(size != 1U ? m_data.ptr + size - 1 : &m_data.val), this};
  }
}
