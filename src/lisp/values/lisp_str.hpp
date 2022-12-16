#pragma once

#include "lisp/values/lisp_object.hpp"
#include "hzd/mem.hpp"
#include "strings/string.hpp"

namespace j::lisp::inline values {
  struct lisp_str final : lisp_object {
    template<Allocator A, typename... Args>
    [[nodiscard]] J_RETURNS_NONNULL static lisp_str * allocate(A & allocator, i32_t size, Args... args)
    {
      void * const ptr = allocator.allocate(8U + align_up(size, 8));
      return ::new (ptr) lisp_str(size, args...);
    }

    template<Allocator A, typename... Args>
    [[nodiscard]] J_RETURNS_NONNULL static lisp_str * allocate(A & allocator, strings::const_string_view value, Args... args)
    {
      void * const ptr = allocator.allocate(8U + align_up(value.size(), 8));
      return ::new (ptr) lisp_str(value, args...);
    }

    J_A(AI,ND) inline lisp_str(i32_t size,
                               lisp::mem::object_hightag_flag flags = (lisp::mem::object_hightag_flag)0U,
                               u8_t ext_refs = 0U) noexcept
      : lisp_object(lisp::mem::str_tag, size, flags, ext_refs)
    { }

    lisp_str(strings::const_string_view value,
             lisp::mem::object_hightag_flag flags = (lisp::mem::object_hightag_flag)0U,
             u8_t ext_refs = 0U) noexcept;

    J_INLINE_GETTER i32_t size() const noexcept {
      return header.lowtag_data();
    }

    J_INLINE_GETTER char * data() noexcept J_RETURNS_ALIGNED(8) {
      return lisp_object::data<char>();
    }

    J_INLINE_GETTER const char * data() const noexcept J_RETURNS_ALIGNED(8) {
      return lisp_object::data<char>();
    }

    [[nodiscard]] strings::const_string_view value() const noexcept {
      return {data(), size()};
    }
  };

  [[nodiscard]] inline bool operator==(const lisp_str & lhs, strings::const_string_view rhs) noexcept {
    i32_t lhs_sz = lhs.size();
    return rhs.size() == lhs_sz && ::j::memcmp(lhs.data(), rhs.data(), lhs_sz) == 0U;
  }

  [[nodiscard]] inline bool operator==(const lisp_str & lhs, const strings::string & rhs) noexcept {
    i32_t lhs_sz = lhs.size();
    return rhs.size() == lhs_sz && ::j::memcmp(lhs.data(), rhs.data(), lhs_sz) == 0U;
  }

  J_INLINE_GETTER_NO_DEBUG bool operator==(const strings::const_string_view & lhs, const lisp_str & rhs) noexcept
  { return operator==(rhs, lhs); }

  J_INLINE_GETTER_NO_DEBUG bool operator==(const strings::string & lhs, const lisp_str & rhs) noexcept
  { return operator==(rhs, lhs); }

  J_INLINE_GETTER bool operator==(const lisp_str & lhs,
                                  const lisp_str & rhs) noexcept
  {
    i32_t lhs_sz = lhs.size();
    return rhs.size() == lhs_sz && (&lhs == &rhs || ::j::memcmp(lhs.data(), rhs.data(), lhs_sz) == 0U);
  }
}
