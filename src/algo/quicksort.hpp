#pragma once

#include "containers/span.hpp"
#include "containers/pair.hpp"

namespace j::algo {
  namespace detail {
    template<typename T>
    J_A(AI,ND,FLATTEN) inline void swap(T * J_NOT_NULL J_RESTRICT a, T * J_NOT_NULL J_RESTRICT b) noexcept {
      T temp(static_cast<T &&>(*a));
      *a = static_cast<T &&>(*b);
      *b = static_cast<T &&>(temp);
    }

    /// Cycle three distinct values around.
    ///
    /// A' <- A <- B <- C <- A
    template<typename T>
    J_A(AI,ND,FLATTEN) inline void swap3(T * J_NOT_NULL J_RESTRICT a,
                                 T * J_NOT_NULL J_RESTRICT b,
                                 T * J_NOT_NULL J_RESTRICT c) noexcept {
      T temp(static_cast<T &&>(*a));
      *a = static_cast<T &&>(*b);
      *b = static_cast<T &&>(*c);
      *c = static_cast<T &&>(temp);
    }

    template<typename T, typename GetKey>
    inline pair<T *, T *> partition(T * J_NOT_NULL start, T * J_NOT_NULL end, GetKey && fn) noexcept {
      --end;
      T * min_pivot = start + (end - start) / 2,
        * max_pivot = min_pivot;

      const auto pivot = static_cast<GetKey &&>(fn)(*min_pivot);

      do {
        for (auto key = static_cast<GetKey &&>(fn)(*start);
             start != min_pivot && !(pivot < key);
             key = static_cast<GetKey &&>(fn)(*start)) {
          if (key == pivot) {
            swap(--min_pivot, start);
          } else {
            ++start;
          }
        }

        for (auto key = static_cast<GetKey &&>(fn)(*end);
             end != max_pivot && !(key < pivot);
             key = static_cast<GetKey &&>(fn)(*end)) {
          if (key == pivot) {
            swap(++max_pivot, end);
          } else {
            --end;
          }
        }

        if (start != min_pivot && end != max_pivot) {
          swap(start, end);
          ++start, --end;
          continue;
        }


        if (start == min_pivot) {
          if (end == max_pivot) {
            return {min_pivot, max_pivot};
          }
          swap3(start, end, ++max_pivot);
          ++start, ++min_pivot;
        } else {
          swap3(end, start, --min_pivot);
          --end, --max_pivot;
        }
      } while (true);
    }

    template<typename T>
    inline pair<T *, T *> partition(T * J_NOT_NULL start, T * J_NOT_NULL end) noexcept {
      --end;
      T * min_pivot = start + (end - start) / 2,
        * max_pivot = min_pivot;

      const T * pivot = min_pivot;

      do {
        while (start != min_pivot && !(*pivot < *start)) {
          if (!(*start < *pivot)) {
            swap(--min_pivot, start);
          } else {
            ++start;
          }
        }

        while (end != max_pivot && !(*end < *pivot)) {
          if (!(*pivot < *end)) {
            swap(++max_pivot, end);
          } else {
            --end;
          }
        }

        if (start != min_pivot && end != max_pivot) {
          swap(start, end);
          ++start, --end;
          continue;
        }


        if (start == min_pivot) {
          if (end == max_pivot) {
            return {min_pivot, max_pivot};
          }
          swap3(start, end, ++max_pivot);
          ++start, ++min_pivot;
        } else {
          swap3(end, start, --min_pivot);
          --end, --max_pivot;
        }
      } while (true);
    }
  }

  template<typename T, typename GetKey>
  inline void quicksort(T * start, T * end, GetKey && fn) noexcept {
    if (start + 2 >= end) {
      if (start + 2 == end &&
          static_cast<GetKey &&>(fn)(start[1]) < static_cast<GetKey &&>(fn)(*start))
      {
        detail::swap(start, start + 1);
      }
      return;
    }

    const auto p = detail::partition(start, end, static_cast<GetKey &&>(fn));
    quicksort(start, p.first, static_cast<GetKey &&>(fn));
    quicksort(p.second + 1, end, static_cast<GetKey &&>(fn));
  }

  template<typename T>
  inline void quicksort(T * start, T * end) noexcept {
    if (start + 2 >= end) {
      if (start + 2 == end && start[1] < *start) {
        detail::swap(start, start + 1);
      }
      return;
    }

    const auto p = detail::partition(start, end);
    quicksort(start, p.first);
    quicksort(p.second + 1, end);
  }

  template<typename T, typename GetKey>
  inline void quicksort(span<T> span, GetKey && get_key) noexcept {
    quicksort(span.begin(), span.end(), static_cast<GetKey &&>(get_key));
  }

  template<typename T>
  inline void quicksort(span<T> span) noexcept {
    quicksort(span.begin(), span.end());
  }
}
