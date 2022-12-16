#pragma once

#include "lisp/cir/ops/atomic_inputs.hpp"
#include "lisp/common/mem_scale.hpp"

namespace j::lisp::cir::inline ops {
  /// Pointer to memory in form `BYTE|WORD|DWORD|QWORD @ [base + index * scale + displacement]`
  struct mem_input final {
    /// The base pointer.
    atomic_input base;
    /// The index added to the pointer, multiplied by scale.
    atomic_input index = {};
    /// Width of the memory access (size of the item targeted by the pointer.)
    mem_width width = mem_width::none;
    /// Multiplier for the index.
    mem_scale scale = mem_scale::byte;
    /// Static byte offset added to the pointer.
    i32_t displacement = 0U;

    J_INLINE_GETTER bool empty() const noexcept {
      return !base;
    }

    J_INLINE_GETTER explicit operator bool() const noexcept {
      return (bool)base;
    }

    J_INLINE_GETTER bool operator!() const noexcept {
      return !base;
    }
  };

  template<typename Base>
  J_INLINE_GETTER constexpr mem_input ptr(mem_width width, Base && base, i32_t displacement = 0) {
    J_ASSUME(width != mem_width::none);
    return {
      .base{static_cast<Base &&>(base)},
      .width = width,
      .displacement = displacement,
    };
  }

  template<typename Base, typename Index>
  J_INLINE_GETTER constexpr mem_input scaled_ptr(mem_width width, Base && base, Index && index, mem_scale scale = mem_scale::width, i32_t displacement = 0) {
    return {
      .base{static_cast<Base &&>(base)},
      .index{static_cast<Index &&>(index)},
      .width = width,
      .scale = scale,
      .displacement = displacement,
    };
  }

  template<typename Base, typename Index>
  J_INLINE_GETTER constexpr mem_input indexed_ptr(mem_width width, Base && base, Index && index, i32_t displacement = 0) {
    return scaled_ptr(width, static_cast<Base &&>(base), static_cast<Index &&>(index), mem_scale::byte, displacement);
  }

  template<typename Base>
  J_INLINE_GETTER constexpr mem_input byte_ptr(Base && base, i32_t displacement = 0) {
    return ptr(mem_width::byte, static_cast<Base &&>(base), displacement);
  }

  template<typename Base, typename Index>
  J_INLINE_GETTER constexpr mem_input indexed_byte_ptr(Base && base, Index && index, i32_t displacement = 0) {
    return indexed_ptr(mem_width::byte, static_cast<Base &&>(base), static_cast<Index &&>(index), displacement);
  }

  template<typename Base, typename Index>
  J_INLINE_GETTER constexpr mem_input scaled_byte_ptr(Base && base, Index && index, mem_scale scale = mem_scale::width, i32_t displacement = 0) {
    return scaled_ptr(mem_width::byte, static_cast<Base &&>(base), static_cast<Index &&>(index), scale, displacement);
  }

  template<typename Base>
  J_INLINE_GETTER constexpr mem_input word_ptr(Base && base, i32_t displacement = 0) {
    return ptr(mem_width::word, static_cast<Base &&>(base), displacement);
  }

  template<typename Base, typename Index>
  J_INLINE_GETTER constexpr mem_input indexed_word_ptr(Base && base, Index && index, i32_t displacement = 0) {
    return indexed_ptr(mem_width::word, static_cast<Base &&>(base), static_cast<Index &&>(index), displacement);
  }

  template<typename Base, typename Index>
  J_INLINE_GETTER constexpr mem_input scaled_word_ptr(Base && base, Index && index, mem_scale scale = mem_scale::width, i32_t displacement = 0) {
    return scaled_ptr(mem_width::word, static_cast<Base &&>(base), static_cast<Index &&>(index), scale, displacement);
  }

  template<typename Base>
  [[nodiscard]] inline constexpr mem_input dword_ptr(Base && base, i32_t displacement = 0) {
    return ptr(mem_width::dword, static_cast<Base &&>(base), displacement);
  }

  template<typename Base, typename Index>
  [[nodiscard]] inline constexpr mem_input indexed_dword_ptr(Base && base, Index && index, i32_t displacement = 0) {
    return indexed_ptr(mem_width::dword, static_cast<Base &&>(base), static_cast<Index &&>(index), displacement);
  }

  template<typename Base, typename Index>
  [[nodiscard]] inline constexpr mem_input scaled_dword_ptr(Base && base, Index && index, mem_scale scale = mem_scale::width, i32_t displacement = 0) {
    return scaled_ptr(mem_width::dword, static_cast<Base &&>(base), static_cast<Index &&>(index), scale, displacement);
  }

  template<typename Base>
  [[nodiscard]] inline constexpr mem_input qword_ptr(Base && base, i32_t displacement = 0) {
    return ptr(mem_width::qword, static_cast<Base &&>(base), displacement);
  }

  template<typename Base, typename Index>
  [[nodiscard]] inline constexpr mem_input indexed_qword_ptr(Base && base, Index && index, i32_t displacement = 0) {
    return indexed_ptr(mem_width::qword, static_cast<Base &&>(base), static_cast<Index &&>(index), displacement);
  }

  template<typename Base, typename Index>
  [[nodiscard]]inline constexpr mem_input scaled_qword_ptr(Base && base, Index && index, mem_scale scale = mem_scale::width, i32_t displacement = 0) {
    return scaled_ptr(mem_width::qword, static_cast<Base &&>(base), static_cast<Index &&>(index), scale, displacement);
  }
}
