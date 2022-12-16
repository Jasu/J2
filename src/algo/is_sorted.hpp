#pragma once

#include "algo/comparer.hpp"
#include "containers/span.hpp"

namespace j::algo {
  namespace detail {
    template<bool Strict, typename It, typename LessThan>
    [[nodiscard]] constexpr bool is_sorted_impl(It begin, It end, LessThan && lt) noexcept {
      if (begin != end) {
        const auto * previous = &*begin;
        while (++begin != end) {
          const auto * const cur = &*begin;
          if constexpr (Strict) {
            if (!static_cast<LessThan &&>(lt)(*previous, *cur))  {
              return false;
            }
          } else {
            if (static_cast<LessThan &&>(lt)(*cur, *previous))  {
              return false;
            }
          }
          previous = cur;
        }
      }

      return true;
    }
  }

  /// Returns true if the range [begin, end) is in non-strict ascending order, false otherwise.
  ///
  /// \param begin Begin iterator of the range to check.
  /// \param end End iterator of the range to check.
  /// \param lt Less-than comparator. Defaults to comparing with the less than operator.
  ///
  /// \return true if [begin, end) is in non-strict ascending order, false otherwise.
  template<typename It, typename LessThan>
  J_A(AI,ND,NODISC) constexpr bool is_sorted(It begin, It end, LessThan && lt = less_than{}) noexcept
  { return detail::is_sorted_impl<false>(begin, end, static_cast<LessThan &&>(lt)); }

  /// Returns true if the span is in non-strict ascending order, false otherwise.
  ///
  /// \param span The span to check.
  /// \param lt Less-than comparator. Defaults to comparing with the less than operator.
  /// \return true if [begin, end) is in non-strict ascending order, false otherwise.
  template<typename T, typename LessThan>
  J_A(AI,ND,NODISC) constexpr bool is_sorted(const span<T> & span,
                                           LessThan && lt = less_than{}) noexcept
  { return detail::is_sorted_impl<false>(span.begin(), span.end(), static_cast<LessThan &&>(lt)); }

  /// Returns true if the range [begin, end) is in a strictly ascending order, false otherwise.
  ///
  /// \param begin Begin iterator of the range to check.
  /// \param end End iterator of the range to check.
  /// \param lt Less-than comparator. Defaults to comparing with the less than operator.
  ///
  /// \return true if [begin, end) is in a strictly ascending order, false otherwise.
  template<typename Begin, typename End, typename LessThan>
  J_A(ND,NODISC,AI) constexpr bool is_strictly_sorted(Begin begin,
                                                      End end,
                                                      LessThan && lt = less_than{}) noexcept
  { return detail::is_sorted_impl<true>(begin, end, static_cast<LessThan &&>(lt)); }

  /// Returns true if the span is in strict ascending order, false otherwise.
  ///
  /// \param span The span to check.
  /// \param lt Less-than comparator. Defaults to comparing with the less than operator.
  /// \return true if [begin, end) is in strict ascending order, false otherwise.
  template<typename T, typename LessThan>
  J_A(ND,NODISC,AI) constexpr bool is_strictly_sorted(const span<T> & span,
                                                      LessThan && lt = less_than{}) noexcept
  { return detail::is_sorted_impl<true>(span.begin(), span.end(), static_cast<LessThan &&>(lt)); }
}

#define J_ASSERT_SORTED(...) J_ASSERT(j::algo::is_sorted(__VA_ARGS__))
#define J_ASSERT_STRICTLY_SORTED(...) J_ASSERT(j::algo::is_strictly_sorted(__VA_ARGS__))
