#pragma once

#include "hzd/iterators.hpp"

namespace j::lisp::cir::inline ssa {
  struct bb;

  template <typename Bb, bool IsReversed>
  struct basic_bb_iterator final {
    Bb * const * it = nullptr;

    J_ALWAYS_INLINE basic_bb_iterator & operator--() noexcept {
      if constexpr (IsReversed) {
        ++it;
      } else {
        --it;
      }
      return *this;
    }

    J_ALWAYS_INLINE basic_bb_iterator operator--(int) noexcept {
      if constexpr (IsReversed) {
        return {++it};
      } else {
        return {--it};
      }
    }

    J_ALWAYS_INLINE basic_bb_iterator & operator++() noexcept {
      if constexpr (IsReversed) {
        --it;
      } else {
        ++it;
      }
      return *this;
    }

    J_ALWAYS_INLINE basic_bb_iterator operator++(int) noexcept {
      if constexpr (IsReversed) {
        return {--it};
      } else {
        return {++it};
      }
    }

    J_INLINE_GETTER_NONNULL Bb * operator->() const noexcept {
      return *it;
    }

    J_INLINE_GETTER Bb & operator*() const noexcept {
      return **it;
    }

    J_INLINE_GETTER bool operator==(const basic_bb_iterator &) const noexcept = default;

    [[nodiscard]] inline operator basic_bb_iterator<const Bb, IsReversed>() const noexcept {
      return {it};
    }
  };

  template<typename Bb, bool IsReversed>
  struct bb_iterator_view final {
    Bb * const * m_begin = nullptr;
    Bb * const * m_end = nullptr;

    J_INLINE_GETTER basic_bb_iterator<Bb, IsReversed> begin() const noexcept {
      if constexpr (IsReversed) {
        return {m_end - 1};
      } else {
        return {m_begin};
      }
    }

    J_INLINE_GETTER basic_bb_iterator<Bb, IsReversed> end() const noexcept {
      if constexpr (IsReversed) {
        return {m_begin - 1};
      } else {
        return {m_end};
      }
    }
  };
}
