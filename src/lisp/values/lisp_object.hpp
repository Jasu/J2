#pragma once

#include "hzd/mem.hpp"
#include "lisp/mem/object_header.hpp"

namespace j::lisp::inline values {
  struct lisp_str;
  struct lisp_vec;
  struct lisp_act_record;

  template<typename O, typename Fn, typename... Args>
  J_ALWAYS_INLINE decltype(auto) visit_lisp_object(O &, Fn &&, Args &&...);

  struct alignas(8) lisp_object {
    J_BOILERPLATE(lisp_object, CTOR_NE_ND, COPY_DEL)

    template<typename... Args>
    J_A(AI,ND,HIDDEN) inline explicit lisp_object(Args ... args) noexcept
      : header(args...)
    { }

    J_A(AI,ND,HIDDEN) inline u32_t lowtag_data() const noexcept {
      return header.lowtag.data();
    }

    template<typename T>
    J_A(AI,ND,HIDDEN,NODISC) inline T * data() noexcept J_RETURNS_ALIGNED(8)
    { return reinterpret_cast<T *>(this + 1); }

    template<typename T>
    J_A(AI,ND,HIDDEN,NODISC) inline const T * data() const noexcept J_RETURNS_ALIGNED(8)
    { return reinterpret_cast<const T *>(this + 1); }

    J_A(AI,ND,HIDDEN,NODISC) inline mem::lowtag_type type() const noexcept
    { return header.lowtag.type(); }

    [[nodiscard]] u32_t size_taken() const noexcept {
      const u32_t data = header.lowtag.data();
      switch (header.lowtag.type()) {
      case mem::lowtag_type::str:
        return align_up(data, 8U) + (data ? 16U : 8U);
      case mem::lowtag_type::vec:
      case mem::lowtag_type::act_record:
        return data * 8U + (data ? 16U : 8U);
      default: J_UNREACHABLE();
      }
    }

    /// Returns whether the object is a string.
    J_INLINE_GETTER bool is_str() const noexcept { return header.lowtag.type() == mem::lowtag_type::str; }
    /// Returns whether the object is a vec (array).
    J_INLINE_GETTER bool is_vec() const noexcept { return header.lowtag.type() == mem::lowtag_type::vec; }
    /// Returns whether the object is an activation record (part of closure-bound stack)
    J_INLINE_GETTER bool is_act_record() const noexcept { return header.lowtag.type() == mem::lowtag_type::act_record; }

    J_INLINE_GETTER lisp_str & as_str() noexcept {
      J_ASSERT(is_str());
      return *reinterpret_cast<lisp_str*>(this);
    }
    J_INLINE_GETTER const lisp_str & as_str() const noexcept
    { return const_cast<lisp_object*>(this)->as_str(); }


    J_INLINE_GETTER lisp_vec & as_vec() noexcept {
      J_ASSERT(is_vec());
      return *reinterpret_cast<lisp_vec*>(this);
    }

    J_INLINE_GETTER const lisp_vec & as_vec() const noexcept
    { return const_cast<lisp_object*>(this)->as_vec(); }

    J_INLINE_GETTER lisp_act_record & as_act_record() noexcept {
      J_ASSERT(is_act_record());
      return *reinterpret_cast<lisp_act_record*>(this);
    }

    J_INLINE_GETTER const lisp_act_record & as_act_record() const noexcept
    { return const_cast<lisp_object*>(this)->as_act_record(); }

    void ext_ref() noexcept {
      header.object_hightag().ext_ref();
    }

    void ext_unref() noexcept {
      header.object_hightag().ext_unref();
    }

    j::lisp::mem::object_header header;

    [[nodiscard]] sources::source_location get_source_location();
  };
}
