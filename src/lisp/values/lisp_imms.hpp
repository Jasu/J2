#pragma once

#include "lisp/values/lisp_imm.hpp"
#include "lisp/values/lisp_vec.hpp"

namespace j::lisp::inline values {
  struct lisp_vec_ref final : lisp_imm {
    J_A(AI,ND) inline explicit lisp_vec_ref(void * J_NOT_NULL vec) noexcept
      : lisp_imm(vec)
    { }

    J_A(AI,ND) inline explicit lisp_vec_ref(lisp_vec * J_NOT_NULL vec) noexcept
      : lisp_imm(reinterpret_cast<u64_t>(vec) + (u64_t)tag_vec_ref)
    { }

    J_A(RNN,NODISC,AI) inline lisp_vec * value() const noexcept {
      return reinterpret_cast<lisp_vec*>(raw - (u64_t)tag_vec_ref);
    }

    J_A(RNN,NODISC,AI) inline lisp_vec * operator->() const noexcept {
      return reinterpret_cast<lisp_vec*>(raw - (u64_t)tag_vec_ref);
    }

    J_A(NODISC,AI) inline lisp_vec & operator*() const noexcept {
      return *reinterpret_cast<lisp_vec*>(raw - (u64_t)tag_vec_ref);
    }

    [[nodiscard]] lisp_imm & operator[](u32_t index) noexcept {
      return (*value())[index];
    }

    [[nodiscard]] const lisp_imm & operator[](u32_t index) const noexcept {
      return (*value())[index];
    }
  };

  struct lisp_str_ref final : lisp_imm {
    J_A(AI,ND) inline explicit lisp_str_ref(void * J_NOT_NULL str) noexcept
      : lisp_imm(str)
    { }

    J_A(AI,ND) inline explicit lisp_str_ref(lisp_str * J_NOT_NULL str) noexcept
      : lisp_imm(reinterpret_cast<u64_t>(str) + (u64_t)tag_str_ref)
    { }

    J_A(AI,RNN,NODISC) inline lisp_str * value() const noexcept {
      return reinterpret_cast<lisp_str*>(raw - (u64_t)tag_str_ref);
    }

    J_A(AI,RNN,NODISC) inline lisp_str * operator->() const noexcept {
      return reinterpret_cast<lisp_str*>(raw - (u64_t)tag_str_ref);
    }

    J_A(AI,NODISC) inline lisp_str & operator*() const noexcept {
      return *reinterpret_cast<lisp_str*>(raw - (u64_t)tag_str_ref);
    }
  };

  struct lisp_sym_id : lisp_imm {
    J_A(AI,ND) inline explicit lisp_sym_id(id value) noexcept
      : lisp_imm{value.raw}
    { }

    [[nodiscard]] id value() const noexcept {
      return bit_cast<id>((u32_t)raw);
    }

    [[nodiscard]] operator id() const noexcept {
      return bit_cast<id>((u32_t)raw);
    }
  };

  struct lisp_fn_ref final : lisp_imm {
    J_A(AI,ND) inline explicit lisp_fn_ref(lisp_fn_ptr_t J_NOT_NULL fn) noexcept
      : lisp_imm{reinterpret_cast<u64_t>(fn) + (u64_t)tag_fn_ref}
    { }

    [[nodiscard]] J_RETURNS_NONNULL lisp_fn_ptr_t value() const noexcept {
      return reinterpret_cast<lisp_fn_ptr_t>(raw - (u64_t)tag_fn_ref);
    }
  };

  struct lisp_closure_ref final : lisp_imm {
    J_A(AI,ND) inline explicit lisp_closure_ref(void * J_NOT_NULL closure_ptr) noexcept
      : lisp_imm{reinterpret_cast<u64_t>(closure_ptr) + (u64_t)tag_closure_ref}
    { }


    J_INLINE_GETTER_NONNULL void * ptr() const noexcept {
      return reinterpret_cast<void*>(raw - (u64_t)tag_closure_ref);
    }

    J_INLINE_GETTER_NONNULL lisp_closure_func_ptr_t function() const noexcept {
      return *reinterpret_cast<lisp_closure_func_ptr_t*>(ptr());
    }

    template<typename... Imms>
    lisp_imm operator()(Imms... imms) const {
      u64_t args[]{imms.raw...};
      return lisp_imm{function()(args, sizeof...(imms), ptr())};
    }

    lisp_imm operator()(imms_t imms) const {
      return lisp_imm{function()((u64_t*)imms.begin(), imms.size(), ptr())};
    }
  };

  struct lisp_f32 final : lisp_imm {
    J_A(AI,ND) explicit inline lisp_f32(float value) noexcept
      : lisp_imm{(u64_t)tag_f32 | ((u64_t)bit_cast<u32_t>(value) << 32)}
    { }

    [[nodiscard]] inline float value() const noexcept {
      return bit_cast<float>((u32_t)(raw >> 32));
    }
  };

  struct lisp_i64 final : lisp_imm {
    J_A(AI,ND) inline explicit lisp_i64(i64_t value) noexcept
      : lisp_imm((u64_t)value << 1U)
    { }

    J_A(AI,NODISC) inline i64_t value() const noexcept {
      return (i64_t)raw >> 1;
    }
  };

  struct lisp_nil final : lisp_imm {
    J_A(AI,ND) inline explicit lisp_nil() noexcept
      : lisp_imm((u64_t)tag_nil)
    { }
  };

  struct lisp_bool final : lisp_imm {
    J_A(AI,ND) inline explicit lisp_bool(bool value) noexcept
      : lisp_imm{(value ? true_v : false_v)}
    { }

    J_A(AI,NODISC) inline bool value() const noexcept {
      return raw == true_v;
    }
  };

  J_A(ND) inline const lisp_nil lisp_nil_v{};
  J_A(ND) inline const lisp_bool lisp_false_v{false};
  J_A(ND) inline const lisp_bool lisp_true_v{true};
}
