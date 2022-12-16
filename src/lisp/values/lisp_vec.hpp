#pragma once

#include "lisp/values/lisp_object.hpp"
#include "lisp/values/lisp_imm.hpp"

namespace j::lisp::inline values {
  struct lisp_vec final : public lisp_object {
    template<Allocator A, typename... Args>
    J_RETURNS_NONNULL static lisp_vec * allocate(A & allocator, u32_t size, Args... args)
    {
      void * const ptr = allocator.allocate(8U + size * 8U);
      return ::new (ptr) lisp_vec(size, args...);
    }

    template<Allocator A, typename... Args>
    J_RETURNS_NONNULL static lisp_vec * allocate(A & allocator, const const_imms_t & value, Args... args)
    {
      void * const ptr = allocator.allocate(8U + value.size() * 8U);
      return ::new (ptr) lisp_vec(value, args...);
    }

    template<Allocator A, u32_t Sz, typename... Args>
    J_A(RNN,AI,HIDDEN) inline static lisp_vec * allocate(A & allocator, const lisp_imm  (& values)[Sz], Args... args)
    {
      void * const ptr = allocator.allocate(8U + Sz * 8U);
      return ::new (ptr) lisp_vec(const_imms_t(values), args...);
    }

    template<Allocator A, typename... Args>
    J_A(RNN) static lisp_vec * allocate_with_debug_info(A & allocator,
                                                        const sources::source_location & loc,
                                                        u32_t size,
                                                        Args...  args)
    {
      void * const ptr = allocator.allocate_with_debug_info(8U + size * 8U, loc);
      return ::new (ptr) lisp_vec(size, args...);
    }

    template<Allocator A, typename... Args>
    J_RETURNS_NONNULL static lisp_vec * allocate_with_debug_info(A & allocator,
                                                                 const sources::source_location & loc,
                                                                 const const_imms_t & value,
                                                                 Args... args)
    {
      void * const ptr = allocator.allocate_with_debug_info(8U + value.size() * 8U, loc);
      return ::new (ptr) lisp_vec(value, args...);
    }

    template<Allocator A, u32_t Sz, typename... Args>
    J_A(AI,ND,HIDDEN) static lisp_vec * allocate_with_debug_info(
      A & allocator, const sources::source_location & loc, const lisp_imm (&values)[Sz], Args... args)
    {
      void * const ptr = allocator.allocate_with_debug_info(8U + Sz * 8U, loc);
      return ::new (ptr) lisp_vec(const_imms_t(values), args...);
    }

    lisp_vec(const const_imms_t & value,
             lisp::mem::object_hightag_flag flags = (lisp::mem::object_hightag_flag)0U,
             u8_t ext_refs = 0U) noexcept;

    J_A(AI,ND) inline lisp_vec(
      u32_t size,
      lisp::mem::object_hightag_flag flags = (lisp::mem::object_hightag_flag)0U,
      u8_t ext_refs = 0U) noexcept
      : lisp_object(lisp::mem::vec_tag, size, flags, ext_refs)
    { }

    J_INLINE_GETTER u32_t size() const noexcept {
      return lowtag_data();
    }

    J_INLINE_GETTER_NONNULL lisp_imm * begin() noexcept {
      return lisp_object::data<lisp_imm>();
    }

    J_INLINE_GETTER_NONNULL const lisp_imm * begin() const noexcept {
      return const_cast<lisp_vec*>(this)->begin();
    }

    J_INLINE_GETTER_NONNULL lisp_imm * end() noexcept {
      return begin() + lowtag_data();
    }

    J_INLINE_GETTER_NONNULL const lisp_imm * end() const noexcept {
      return const_cast<lisp_vec*>(this)->end();
    }

    J_INLINE_GETTER bool empty() const noexcept {
      return !lowtag_data();
    }

    J_INLINE_GETTER explicit operator bool() const noexcept {
      return lowtag_data();
    }

    J_INLINE_GETTER bool operator!() const noexcept {
      return !lowtag_data();
    }

    [[nodiscard]] lisp_imm & at(u32_t i) {
      J_REQUIRE(i < size(), "Index out of range");
      return lisp_object::data<lisp_imm>()[i];
    }

    J_INLINE_GETTER lisp_imm at(u32_t i) const noexcept {
      return const_cast<lisp_vec*>(this)->at(i);
    }

    J_INLINE_GETTER lisp_imm & operator[](u32_t i) noexcept {
      return lisp_object::data<lisp_imm>()[i];
    }

    J_INLINE_GETTER const lisp_imm & operator[](u32_t i) const noexcept {
      return lisp_object::data<const lisp_imm>()[i];
    }

    [[nodiscard]] const_imms_t value() const noexcept {
      return {lisp_object::data<const lisp_imm>(), (i32_t)lowtag_data()};
    }

    [[nodiscard]] imms_t value() noexcept {
      return {lisp_object::data<lisp_imm>(), (i32_t)lowtag_data()};
    }
  };
}
