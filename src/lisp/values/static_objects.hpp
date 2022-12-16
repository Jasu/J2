#pragma once

#include "lisp/values/lisp_imms.hpp"
#include "lisp/values/lisp_vec.hpp"
#include "lisp/values/lisp_str.hpp"

namespace j::lisp::inline values {
  template<u32_t Size>
  struct static_lisp_vec final {
    template<typename... Imms>
    explicit consteval static_lisp_vec(Imms... imms) noexcept
      : value(sizeof...(Imms), lisp::mem::object_hightag_flag::non_gc, 1U),
        data{imms...}
    { }

    union {
      lisp_vec value;
      char value_bytes[8];
    };
    lisp_imm data[Size];

    J_A(NODISC,AI,HIDDEN) inline operator lisp_vec_ref() const noexcept {
      return lisp_vec_ref((void*)&value_bytes[(u8_t)imm_vec_ref]);
    }

    J_A(NODISC,AI,HIDDEN) inline operator lisp_imm() const noexcept {
      return lisp_imm((void*)&value_bytes[(u8_t)imm_vec_ref]);
    }
  };

  template<typename... Imms>
  static_lisp_vec(Imms...) -> static_lisp_vec<sizeof...(Imms)>;

  template<u32_t Size>
  struct static_lisp_str final {
    explicit consteval static_lisp_str(const char (&str)[Size + 1]) noexcept
      : header(lisp::mem::str_tag, Size, lisp::mem::object_hightag_flag::non_gc, 1)
    {
      __builtin_memcpy(m_str, &str[0], Size);
    }

    union {
      j::lisp::mem::object_header header;
      j::lisp::lisp_str value;
      char value_bytes[8];
    };
    char m_str[j::align_up(Size, 8U)] = { 0 };
    J_A(AI,NODISC,HIDDEN) inline operator lisp_str_ref() const noexcept {
      return lisp_str_ref((void*)&value_bytes[(u8_t)tag_str_ref]);
    }

    J_A(AI,NODISC,HIDDEN) inline operator lisp_imm() const noexcept {
      return lisp_imm((void*)&value_bytes[(u8_t)tag_str_ref]);
    }
  };

  template<u32_t Sz>
  explicit static_lisp_str(const char (&str)[Sz]) -> static_lisp_str<Sz - 1>;
}
