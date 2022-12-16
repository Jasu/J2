#pragma once

#include "hzd/utility.hpp"

namespace j::lisp::cir::inline ssa {
  template<typename Op, bool IsReverse>
  struct bb_op_iterator final {
    J_INLINE_GETTER_NONNULL Op * operator->() const noexcept {
      return m_it;
    }

    J_INLINE_GETTER Op & operator*() const noexcept {
      return *m_it;
    }

    J_INLINE_GETTER bool operator!() const noexcept {
      return !m_it;
    }

    J_INLINE_GETTER explicit operator bool() const noexcept {
      return m_it;
    }

    bb_op_iterator & operator++() noexcept {
      m_it = m_next;
      if (m_it) {
        if constexpr (IsReverse) {
          m_next = m_it->previous;
        } else {
          m_next = m_it->next;
        }
      }
      return *this;
    }

    bb_op_iterator operator++(int) noexcept {
      bb_op_iterator result = *this;
      operator++();
      return result;
    }

    J_INLINE_GETTER bool operator==(const bb_op_iterator & rhs) const noexcept {
      return m_it == rhs.m_it;
    }

    Op * m_it = nullptr;
    /// Pointer to the next iteratee.
    ///
    /// \note This is stored when incrementing, to keep the iterators valid if an op is
    /// removed, inserted, or replaced at the current position.
    Op * m_next = nullptr;
  };
}
