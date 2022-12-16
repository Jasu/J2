#pragma once

#include "lisp/values/lisp_str.hpp"
#include "strings/string_hash.hpp"

namespace j::lisp::inline values {
  struct lisp_str_hash final {
    J_A(ND,FLATTEN) u32_t operator()(const lisp_str & str) const noexcept {
      return strings::hash_string(str.data(), str.size());
    }

    template<typename T>
    J_A(ND,FLATTEN) u32_t operator()(const T & str) const noexcept {
      return strings::hash_string<T>(str);
    }
  };

  struct lisp_str_ptr_hash final {
    J_A(ND,FLATTEN) u32_t operator()(const lisp_str * J_NOT_NULL str) const noexcept {
      return strings::hash_string(str->data(), str->size());
    }

    template<typename T>
    J_A(ND,FLATTEN) u32_t operator()(const T & str) const noexcept {
      return strings::hash_string<T>(str);
    }
  };

  struct lisp_str_ptr_equal_to final {
    J_A(ND,FLATTEN) bool operator()(const lisp_str * J_NOT_NULL lhs,
                                    const lisp_str * J_NOT_NULL rhs) const noexcept {
      return *lhs == *rhs;
    }

    J_A(ND,FLATTEN) bool operator()(const lisp_str * J_NOT_NULL lhs,
                                    const strings::const_string_view & rhs) const noexcept {
      return *lhs == rhs;
    }
  };

  struct lisp_str_ptr_const_key final {
    using type = strings::const_string_view;
    using arg_type = strings::const_string_view;
    using getter_type = strings::const_string_view;

    J_A(AI,NODISC) inline strings::const_string_view operator()(const lisp_str * J_NOT_NULL value) const noexcept {
      return value->value();
    }
  };
}
