#pragma once

#include "lisp/common/id.hpp"
#include "lisp/cir/ops/mem_input.hpp"

namespace j::lisp::cir::inline ops {
  enum class input_type : u8_t {
    none = 0U,

    op_result,
    reloc,
    constant,
    mem,
  };

  inline constexpr u8_t num_input_types = (u8_t)input_type::mem + 1U;

  struct input final {
    J_A(AI) inline constexpr input() noexcept : type(input_type::none), op_result_data{} { }

    input(const atomic_input & i) noexcept;

    input(const input & i) noexcept {
      ::j::memcpy(this, &i, sizeof(input));
    }

    input(input && i) noexcept {
      ::j::memcpy(this, &i, sizeof(input));
    }

    input & operator=(const input & i) noexcept {
      ::j::memcpy(this, &i, sizeof(input));
      return *this;
    }

    input & operator=(input && i) noexcept {
      ::j::memcpy(this, &i, sizeof(input));
      return *this;
    }

    inline constexpr input(const mem_input & m) noexcept
      : type(input_type::mem),
        mem_data{m}
    {
    }

    inline constexpr input(op * J_NOT_NULL o) noexcept
      : type(input_type::op_result),
        op_result_data{o}
    {
    }

    inline constexpr input(const op_result_input & result) noexcept
      : type(input_type::op_result),
        op_result_data{result}
    {
    }

    explicit inline constexpr input(u64_t c, imm_type type) noexcept
      : type(input_type::constant),
        types(type),
        const_data{c}
    {
    }

    inline constexpr input(const reloc_input & reloc) noexcept
      : type(input_type::reloc),
        reloc_data{reloc}
    {
    }

    J_INLINE_GETTER bool is_op_result() const noexcept {
      return type == input_type::op_result;
    }

    J_INLINE_GETTER bool is_reloc() const noexcept {
      return type == input_type::reloc;
    }

    J_INLINE_GETTER bool is_fn_addr() const noexcept {
      return type == input_type::reloc && reloc_data.reloc_type == reloc_type::fn_addr;
    }

    J_INLINE_GETTER bool is_full_call_addr() const noexcept {
      return is_fn_addr() && reloc_data.calling_convention == functions::calling_convention::full_call;
    }

    J_INLINE_GETTER bool is_abi_call_addr() const noexcept {
      return is_fn_addr() && reloc_data.calling_convention == functions::calling_convention::abi_call;
    }

    J_INLINE_GETTER bool is_var_addr() const noexcept {
      return type == input_type::reloc && reloc_data.reloc_type == reloc_type::var_addr;
    }

    J_INLINE_GETTER bool is_untagged_var_value() const noexcept {
      return type == input_type::reloc && reloc_data.reloc_type == reloc_type::var_value_untagged;
    }

    J_INLINE_GETTER bool is_tagged_var_value() const noexcept {
      return type == input_type::reloc && reloc_data.reloc_type == reloc_type::var_value_tagged;
    }

    J_INLINE_GETTER bool is_mem() const noexcept {
      return type == input_type::mem;
    }

    J_INLINE_GETTER bool is_constant() const noexcept {
      return type == input_type::constant;
    }

    [[nodiscard]] explicit inline operator atomic_input() const noexcept {
      switch (type) {
      case input_type::none:
        return {};
      case input_type::op_result:
        return {op_result_data};
      case input_type::reloc:
        return {reloc_data};
      case input_type::constant:
        return atomic_input(const_data, types);
      case input_type::mem:
        J_FAIL("Tried to convert mem op to atomic.");
      }
    }

    J_A(AI,NODISC) inline bool empty() const noexcept {
      return type == input_type::none;
    }

    J_A(AI,NODISC) inline explicit operator bool() const noexcept {
      return type != input_type::none;
    }

    J_A(AI,NODISC) inline bool operator!() const noexcept {
      return type == input_type::none;
    }
    input_type type = input_type::none;
    imm_type_mask types{};

    union {
      op_result_input op_result_data = {};
      reloc_input reloc_data;
      u64_t const_data;
      mem_input mem_data;
    };

    input copy_value() noexcept;
    void set_value(u8_t index, const input & in) noexcept;
    void attach(u8_t index) noexcept;
    void detach() noexcept;

    void verify_attached(op * J_NOT_NULL target) noexcept;
  };
}
