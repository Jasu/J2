#pragma once

#include "util/flags.hpp"
#include "lisp/common/truthiness.hpp"

namespace j::lisp::inline values {
  enum imm_type : u8_t {
    imm_i64, ///< Integer (63-bit really, as the tag bit is zero)
    imm_f32, ///< Single-precision floating point.

    imm_bool, ///< True or false.
    imm_nil, ///< Nil (single value in type)

    imm_sym_id, ///< Reference to a symbol.
    imm_fn_ref, ///< Pointer to a function.
    imm_closure_ref, ///< Pointer to a closure.
    imm_str_ref, ///< Pointer to a string.
    imm_vec_ref, ///< Pointer to a vector.
    imm_rec_ref, ///< Pointer to any rec that has no special tag.
    imm_act_rec, ///< Activation record pointer.
    imm_range, ///< Pointer + number of elements.
    imm_undefined, ///< During compilation passes, indicates a "null" lisp_imm.

    imm_max_numeric   = imm_f32,
    imm_max_non_ptr   = imm_sym_id,
    imm_max_tagged    = imm_rec_ref,
    imm_max_storeable = imm_act_rec,
    imm_max           = imm_undefined,
  };

  constexpr inline u8_t num_imm_types = imm_max + 1U;

  using imm_type_mask = util::flags<imm_type, u16_t>;

  J_FLAG_OPERATORS(imm_type, u16_t)

  /// Mask contining `imm_f32` and `imm_i64`.
  //                                                                       Bool╾╮
  //                                                                      Nil╾╮ │ ╭╼F32
  //                                                                    Sym╾╮ │ │ │ ╭╼I64
  //                                                                        ╽ ╽ ╽ ╽ ╽
  constexpr inline imm_type_mask numeric_imm_type   {0b000000000011U};
  /// Mask containing potentially falsy immediate types `imm_bool` and `imm_nil`.
  constexpr inline imm_type_mask boolean_imm_type   {0b000000001100U};
  /// Mask containing `imm_sym`, `imm_nil`, `imm_bool`, `imm_f32`, and `imm_i64`.
  constexpr inline imm_type_mask non_ptr_imm_type   {0b000000011111U};

  constexpr inline imm_type_mask taggable_imm_type  {0b001111111111U};

  /// Mask of callable immediate types `imm_closure` and `imm_fn`.
  constexpr inline imm_type_mask callable_imm_type  {0b000001100000U};

  /// Mask of all immediate types, including rest arguments (`imm_range`).
  //                                                ActRec╾╮
  //                                               Range╾╮ │
  //                                                     ╽ ╽
  constexpr inline imm_type_mask any_imm_type       {0b111111111111U};
  /// Mask of range immediate types, `imm_vec` and `imm_va_args`.
  constexpr inline imm_type_mask range_imm_type     {0b100100000000U};

  J_A(AI,NODISC) constexpr inline truthiness type_truthiness(imm_type t) noexcept {
    if (t == imm_bool) {
      return truthiness::unknown;
    }
    return t == imm_nil ? truthiness::always_falsy : truthiness::always_truthy;
  }

  J_A(NODISC,CONST) truthiness type_truthiness(const imm_type_mask & m) noexcept;

  /// Tags of immediates.
  enum imm_tag : u64_t {
    tag_i64         =        0b0U, ///< Integer (63-bit really, tag bit is always zero)
    tag_vec_ref     =      0b001U, ///< Pointer to a vector.
    tag_closure_ref =      0b011U, ///< Pointer to a closure.
    tag_str_ref     =      0b101U, ///< Pointer to a string.
    tag_rec_ref     =   0b000111U, ///< Pointer to any rec that has no special tag.
    tag_fn_ref      =   0b001111U, ///< A non-closure function.
    tag_bool        =   0b010111U, ///< True or false (next bit decides)
    tag_nil         =   0b011111U, ///< Nil (single value in type)
    tag_sym_id      =   0b100111U, ///< 32-bit symbol index
    tag_f32         =   0b101111U, ///< Single-precision (32-bit) floating point.
    tag_undefined   = 0b11111111U, ///< During compilation passes, indicates a "null" lisp_imm.
  };


  J_A(CONST,NODISC) imm_tag imm_tag_by_type(imm_type idx) noexcept;

  J_A(CONST,NODISC) imm_type imm_type_by_tag(imm_tag t) noexcept;

  J_A(ND) constexpr inline u64_t nil_v       = (u64_t)tag_nil;
  J_A(ND) constexpr inline u64_t false_v     = (u64_t)tag_bool;
  J_A(ND) constexpr inline u64_t true_v      = (u64_t)tag_bool | 0b01000000U;
  J_A(ND) constexpr inline u64_t undefined_v = (u64_t)tag_undefined;

  J_A(NODISC,CONST) imm_tag tag_of(u8_t tagged) noexcept;

  J_A(AI,NODISC) constexpr inline bool is_i64(u8_t tagged) noexcept
  { return !(tagged & 1); }

  J_A(AI,NODISC) constexpr inline bool is_vec_ref(u8_t tagged) noexcept
  { return (tagged & 0b111U) == (u8_t)tag_vec_ref; }
  J_A(AI,NODISC) constexpr inline bool is_closure_ref(u8_t tagged) noexcept
  { return (tagged & 0b111U) == (u8_t)tag_closure_ref; }
  J_A(AI,NODISC) constexpr inline bool is_str_ref(u8_t tagged) noexcept
  { return (tagged & 0b111U) == (u8_t)tag_str_ref; }


  J_A(AI,NODISC) inline constexpr bool is_rec_ref(u8_t tagged) noexcept
  { return (tagged & 0b111111U) == (u8_t)tag_rec_ref; }
  J_A(AI,NODISC) inline constexpr bool is_fn_ref(u8_t tagged) noexcept
  { return (tagged & 0b111111U) == (u8_t)tag_fn_ref; }
  J_A(AI,NODISC) inline constexpr bool is_sym_id(u8_t tagged) noexcept
  { return (tagged & 0b111111U) == (u8_t)tag_sym_id; }
  J_A(AI,NODISC) inline constexpr bool is_bool(u8_t tagged) noexcept
  { return (tagged & 0b111111U) == (u8_t)tag_bool; }
  J_A(AI,NODISC) inline constexpr bool is_nil(u8_t tagged) noexcept
  { return tagged == (u8_t)tag_nil; }
  J_A(AI,NODISC) inline constexpr bool is_f32(u8_t tagged) noexcept
  { return (tagged & 0b111111U) == (u8_t)tag_f32; }
  J_A(AI,NODISC) inline constexpr bool is_undefined(u8_t tagged) noexcept
  { return tagged == tag_undefined; }
}


J_DECLARE_EXTERN_FLAGS(j::lisp::values::imm_type, u16_t);
