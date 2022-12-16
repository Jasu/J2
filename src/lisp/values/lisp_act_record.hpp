#pragma once

#include "lisp/values/lisp_object.hpp"
#include "lisp/values/lisp_imms.hpp"

namespace j::lisp::inline values {
  struct lisp_act_record final : public lisp_object {
  public:
    template<Allocator A, typename... Args>
    [[nodiscard]] J_RETURNS_NONNULL static lisp_act_record * allocate(A & allocator, u32_t num_closures, u32_t num_bindings, Args && ... args)
    {
      void * const ptr = allocator.allocate(8U + 8U + (num_closures + num_bindings) * 8U);
      return ::new (ptr) lisp_act_record(8U, 1 + 1 + num_closures + num_bindings, static_cast<Args &&>(args)...);
    }

    template<Allocator A, typename... Args>
    [[nodiscard]] J_RETURNS_NONNULL static lisp_act_record * allocate(A & allocator, lisp_act_record * prev, span<void*> closures, const_imms_t value, Args && ... args)
    {
      void * const ptr = allocator.allocate(8U + 8U + (closures.size() + value.size()) * 8U);
      return ::new (ptr) lisp_act_record(prev, closures, value, static_cast<Args &&>(args)...);
    }

    lisp_act_record(lisp_act_record * prev, span<void*> closures, const_imms_t bindings,
                                    lisp::mem::object_hightag_flag flags = (lisp::mem::object_hightag_flag)0U,
                             u8_t ext_refs = 0U) noexcept
      : lisp_object(lisp::mem::act_record_tag, closures.size() + bindings.size() + 1, flags, ext_refs)
    {
      auto at = lisp_object::data<u64_t>();
      *at++ = (u64_t)prev;

      if (const u32_t sz = closures.size()) {
        ::j::memcpy(at, closures.begin(), sz * 8U);
        at += sz;
      }
      ::j::memcpy(at, bindings.begin(), bindings.size() * 8U);
    }

    J_A(AI,ND) lisp_act_record(u32_t size,
                    lisp::mem::object_hightag_flag flags = (lisp::mem::object_hightag_flag)0U,
                    u8_t ext_refs = 0U) noexcept
      : lisp_object(lisp::mem::act_record_tag, size, flags, ext_refs)
    {
      ::j::memzero(lisp_object::data<void>(), size * 8U);
    }

    [[nodiscard]] lisp_act_record * prev() const noexcept {
      return *lisp_object::data<lisp_act_record*>();
    }

    [[nodiscard]] span<void *> closures(i32_t num_closures) noexcept {
      return { lisp_object::data<void*>() + 1, num_closures };
    }

    [[nodiscard]] imms_t closures(i32_t num_closures, i32_t num_bindings) noexcept {
      return { lisp_object::data<lisp_imm>() + 1 + num_closures, num_bindings };
    }

    J_INLINE_GETTER u32_t size() const noexcept {
      return lowtag_data();
    }
  };
}
