#pragma once

#include "exceptions/assert_lite.hpp"
#include "hzd/type_traits.hpp"
#include "hzd/string.hpp"

namespace j::algo {
  enum class copy_tag_t { v };
  enum class move_tag_t { v };
  enum class initialize_tag_t { v };
  enum class no_initialize_tag_t { v };
  enum class destroy_tag_t { v };
  enum class no_destroy_tag_t { v };

  constexpr inline copy_tag_t          copy_tag          = copy_tag_t::v;
  constexpr inline move_tag_t          move_tag          = move_tag_t::v;
  constexpr inline initialize_tag_t    initialize_tag    = initialize_tag_t::v;
  constexpr inline no_initialize_tag_t no_initialize_tag = no_initialize_tag_t::v;
  constexpr inline destroy_tag_t       destroy_tag       = destroy_tag_t::v;
  constexpr inline no_destroy_tag_t    no_destroy_tag    = no_destroy_tag_t::v;

  namespace detail {
    template<typename CopyTag, typename InitTag, typename DestroyTag, typename T, typename U>
    void copy_single(CopyTag, InitTag, DestroyTag, T * J_NOT_NULL J_RESTRICT to, U * J_NOT_NULL J_RESTRICT from) noexcept
    {
      if constexpr (is_same_v<InitTag, initialize_tag_t>) {
        if constexpr (is_same_v<CopyTag, copy_tag_t>) {
          ::new (to) T(*from);
        } else {
          static_assert(is_same_v<CopyTag, move_tag_t>);
          ::new (to) T(static_cast<U &&>(*from));
        }
      } else {
        static_assert(is_same_v<InitTag, no_initialize_tag_t>);
        if constexpr (is_same_v<CopyTag, copy_tag_t>) {
          *to = *from;
        } else {
          static_assert(is_same_v<CopyTag, move_tag_t>);
          *to = static_cast<U &&>(*from);
        }
      }
      if constexpr (is_same_v<DestroyTag, destroy_tag_t>) {
        from->~U();
      } else {
        static_assert(is_same_v<DestroyTag, no_destroy_tag_t>);
      }
    }

    template<typename CopyTag, typename InitTag, typename DestroyTag, typename T, typename U>
    void copy_noalias(CopyTag ct, InitTag it, DestroyTag dt,
                      T * J_NOT_NULL J_RESTRICT to,
                      U * J_NOT_NULL J_RESTRICT from,
                      u32_t num) noexcept {
      J_ASSUME(num > 0);
      if constexpr (is_trivially_copyable_v<T>) {
        ::j::memcpy(to, from, num * sizeof(T));
      } else {
        for (u32_t i = 0U; i < num; ++to, ++from, ++num) {
          copy_single(ct, it, dt, to, from);
        }
      }
    }
  }

  template<typename T>
  void move_initialize_destroy(T * J_NOT_NULL J_RESTRICT to,
                               T * J_NOT_NULL J_RESTRICT from,
                               u32_t num) noexcept {
    detail::copy_noalias(move_tag, initialize_tag, destroy_tag, to, from, num);
  }

  template<typename T>
  void move_forward(T * J_NOT_NULL to, T * J_NOT_NULL from, u32_t num) noexcept {
    J_ASSUME(to > from && num);
    if constexpr (is_trivially_copyable_v<T>) {
      ::j::memmove(to, from, num * sizeof(T));
    } else {
      const T * const first = to;
      to += num;
      from += num;
      do {
        *--to = static_cast<T &&>(*--from);
      } while (to != first);
    }
  }
}
