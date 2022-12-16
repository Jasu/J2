#pragma once

#include "hzd/concepts.hpp"
#include "hzd/type_traits.hpp"
#include "strings/string_view.hpp"
#include "strings/string.hpp"
#include "lisp/values/lisp_imms.hpp"
#include "lisp/values/lisp_str.hpp"

namespace j::lisp::functions {
  template<typename Arg> struct arg_parsers;
  template<Integral Arg> struct arg_parsers<Arg> {
    static Arg parse(lisp_imm imm) {
      return imm.as_i64().value();
    }
  };
  template<> struct arg_parsers<bool> {
    static bool parse(lisp_imm imm) {
      return imm.as_bool().value();
    }
  };

  template<> struct arg_parsers<float> {
    static float parse(lisp_imm imm) {
      return imm.as_f32().value();
    }
  };

  template<> struct arg_parsers<double> {
    static double parse(lisp_imm imm) {
      return imm.as_f32().value();
    }
  };

  template<> struct arg_parsers<id> {
    static id parse(lisp_imm imm) {
      return imm.as_sym_id().value();
    }
  };

  template<> struct arg_parsers<imms_t> {
    static imms_t parse(lisp_imm imm) {
      return imm.as_vec_ref()->value();
    }
  };

  template<> struct arg_parsers<const_imms_t> {
    static const_imms_t parse(lisp_imm imm) {
      return imm.as_vec_ref()->value();
    }
  };

  template<> struct arg_parsers<lisp_str &> {
    static lisp_str & parse(lisp_imm imm) {
      return *imm.as_str_ref().value();
    }
  };

  template<> struct arg_parsers<const lisp_str &> {
    static const lisp_str & parse(lisp_imm imm) {
      return *imm.as_str_ref().value();
    }
  };

  template<> struct arg_parsers<strings::const_string_view> {
    static strings::const_string_view parse(lisp_imm imm) {
      return imm.as_str_ref()->value();
    }
  };

  template<> struct arg_parsers<strings::string> {
    static strings::string parse(lisp_imm imm) {
      return imm.as_str_ref()->value();
    }
  };

  inline lisp_imm encode_value(const lisp_imm & v) {
    return v;
  }

  inline lisp_imm encode_value(lisp_str & v) {
    return lisp_str_ref(&v);
  }

  inline lisp_imm encode_value(bool v) {
    return lisp_bool(v);
  }

  template<Integral T>
  lisp_imm encode_value(T v) {
    return lisp_i64(v);
  }

  template<typename T>
  lisp_imm encode_value(T * v) {
    return v ? lisp_i64((u64_t)(void*)v) : lisp_nil{};
  }

  template<typename FnT, FnT Fn> struct foreign_function_wrapper;
  template<typename Ret, typename... Args, Ret (*Fn) (Args...)>
  struct foreign_function_wrapper<Ret (*) (Args...), Fn> {
    static u64_t call(u64_t * args, u32_t num_args) {
      J_ASSERT(num_args == sizeof...(Args), "Argument count mismatch");
      J_ASSERT(num_args == 0 || args != nullptr, "Argument array was null");
      if constexpr (is_void_v<Ret>) {
        Fn(arg_parsers<Args>::parse(lisp_imm{*args++})...);
        return nil_v;
      } else {
        return encode_value(Fn(arg_parsers<Args>::parse(lisp_imm{*args++})...));
      }
    }
  };
}
