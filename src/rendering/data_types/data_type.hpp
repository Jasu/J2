#pragma once

#include "hzd/mem.hpp"

namespace j::rendering::data_types {
  /// Mask for byte size in data type enums.
  inline constexpr u8_t element_size_mask = 3U;

  inline constexpr u8_t elem_8 = 0U;
  inline constexpr u8_t elem_16 = 1U;
  inline constexpr u8_t elem_32 = 2U;
  inline constexpr u8_t elem_64 = 3U;

  inline constexpr u8_t col_shift = 2U;
  inline constexpr u8_t col_mask = 3U << col_shift;
  inline constexpr u8_t row_shift = 4U;
  inline constexpr u8_t row_mask = 3U << row_shift;
  inline constexpr u8_t shape_mask = col_mask | row_mask;

  inline constexpr u8_t shape(u8_t cols, u8_t rows) noexcept {
    return ((cols - 1U) << col_shift) | ((rows - 1U) << row_shift);
  }

  inline constexpr u8_t shape_scalar = 0U;

  inline constexpr u8_t shape_vec2 = 0b000100U;
  inline constexpr u8_t shape_vec3 = 0b001000U;
  inline constexpr u8_t shape_vec4 = 0b001100U;

  inline constexpr u8_t shape_mat2x2 = 0b010100U;
  inline constexpr u8_t shape_mat2x3 = 0b011000U;
  inline constexpr u8_t shape_mat2x4 = 0b011100U;

  inline constexpr u8_t shape_mat3x2 = 0b100100U;
  inline constexpr u8_t shape_mat3x3 = 0b101000U;
  inline constexpr u8_t shape_mat3x4 = 0b101100U;

  inline constexpr u8_t shape_mat4x2 = 0b110100U;
  inline constexpr u8_t shape_mat4x3 = 0b111000U;
  inline constexpr u8_t shape_mat4x4 = 0b111100U;

  inline constexpr u8_t type_shift = 6U;
  inline constexpr u8_t type_mask = 3U << type_shift;

  inline constexpr u8_t type_unsigned = 0U;
  inline constexpr u8_t type_signed = 1U << type_shift;
  inline constexpr u8_t type_boolean = 2U << type_shift;
  inline constexpr u8_t type_fp = 3U << type_shift;

  /// Enum of integer / float / vector data types.
  enum class data_type : u8_t {
    none = 0U,

    u8       = elem_8,
    u16      = elem_16,
    u32      = elem_32,
    u64      = elem_64,

    s8       = elem_8 | type_signed,
    s16      = elem_16 | type_signed,
    s32      = elem_32 | type_signed,
    s64      = elem_64 | type_signed,

    fp16     = elem_16 | type_fp,
    fp32     = elem_32 | type_fp,
    fp64     = elem_64 | type_fp,

    boolean  = elem_32 | type_boolean,



    vec2u8   =             shape_vec2,
    vec2u16  = elem_16   | shape_vec2,
    vec2u32  = elem_32   | shape_vec2,
    vec2u64  = elem_64   | shape_vec2,

    vec2s8   =             shape_vec2   | type_signed,
    vec2s16  = elem_16   | shape_vec2   | type_signed,
    vec2s32  = elem_32   | shape_vec2   | type_signed,
    vec2s64  = elem_64   | shape_vec2   | type_signed,

    vec2fp16 = elem_16   | shape_vec2   | type_fp,
    vec2fp32 = elem_32   | shape_vec2   | type_fp,
    vec2fp64 = elem_64   | shape_vec2   | type_fp,

    vec2b    = elem_32  | shape_vec2   | type_boolean,


    vec3u8   =             shape_vec3,
    vec3u16  = elem_16   | shape_vec3,
    vec3u32  = elem_32   | shape_vec3,
    vec3u64  = elem_64   | shape_vec3,

    vec3s8   =             shape_vec3   | type_signed,
    vec3s16  = elem_16   | shape_vec3   | type_signed,
    vec3s32  = elem_32   | shape_vec3   | type_signed,
    vec3s64  = elem_64   | shape_vec3   | type_signed,

    vec3fp16 = elem_16   | shape_vec3   | type_fp,
    vec3fp32 = elem_32   | shape_vec3   | type_fp,
    vec3fp64 = elem_64   | shape_vec3   | type_fp,

    vec3b    = elem_32  | shape_vec3   | type_boolean,


    vec4u8   =             shape_vec4,
    vec4u16  = elem_16   | shape_vec4,
    vec4u32  = elem_32   | shape_vec4,
    vec4u64  = elem_64   | shape_vec4,

    vec4s8   =             shape_vec4   | type_signed,
    vec4s16  = elem_16   | shape_vec4   | type_signed,
    vec4s32  = elem_32   | shape_vec4   | type_signed,
    vec4s64  = elem_64   | shape_vec4   | type_signed,

    vec4fp16 = elem_16   | shape_vec4   | type_fp,
    vec4fp32 = elem_32   | shape_vec4   | type_fp,
    vec4fp64 = elem_64   | shape_vec4   | type_fp,

    vec4b    = elem_32  | shape_vec4   | type_boolean,


    mat2x2u8   =           shape_mat2x2,
    mat2x2u16  = elem_16 | shape_mat2x2,
    mat2x2u32  = elem_32 | shape_mat2x2,
    mat2x2u64  = elem_64 | shape_mat2x2,

    mat2x2s8   = elem_8  | shape_mat2x2 | type_signed,
    mat2x2s16  = elem_16 | shape_mat2x2 | type_signed,
    mat2x2s32  = elem_32 | shape_mat2x2 | type_signed,
    mat2x2s64  = elem_64 | shape_mat2x2 | type_signed,

    mat2x2fp16 = elem_16 | shape_mat2x2 | type_fp,
    mat2x2fp32 = elem_32 | shape_mat2x2 | type_fp,
    mat2x2fp64 = elem_64 | shape_mat2x2 | type_fp,

    mat2x2b    = elem_32 | shape_mat2x2 | type_boolean,


    mat3x2u8   =           shape_mat3x2,
    mat3x2u16  = elem_16 | shape_mat3x2,
    mat3x2u32  = elem_32 | shape_mat3x2,
    mat3x2u64  = elem_64 | shape_mat3x2,

    mat3x2s8   = elem_8  | shape_mat3x2 | type_signed,
    mat3x2s16  = elem_16 | shape_mat3x2 | type_signed,
    mat3x2s32  = elem_32 | shape_mat3x2 | type_signed,
    mat3x2s64  = elem_64 | shape_mat3x2 | type_signed,

    mat3x2fp16 = elem_16 | shape_mat3x2 | type_fp,
    mat3x2fp32 = elem_32 | shape_mat3x2 | type_fp,
    mat3x2fp64 = elem_64 | shape_mat3x2 | type_fp,

    mat3x2b    = elem_32 | shape_mat3x2 | type_boolean,


    mat4x2u8   =           shape_mat4x2,
    mat4x2u16  = elem_16 | shape_mat4x2,
    mat4x2u32  = elem_32 | shape_mat4x2,
    mat4x2u64  = elem_64 | shape_mat4x2,

    mat4x2s8   =           shape_mat4x2 | type_signed,
    mat4x2s16  = elem_16 | shape_mat4x2 | type_signed,
    mat4x2s32  = elem_32 | shape_mat4x2 | type_signed,
    mat4x2s64  = elem_64 | shape_mat4x2 | type_signed,

    mat4x2fp16 = elem_16 | shape_mat4x2 | type_fp,
    mat4x2fp32 = elem_32 | shape_mat4x2 | type_fp,
    mat4x2fp64 = elem_64 | shape_mat4x2 | type_fp,

    mat4x2b    = elem_32 | shape_mat4x2 | type_boolean,


    mat2x3u8   =           shape_mat2x3,
    mat2x3u16  = elem_16 | shape_mat2x3,
    mat2x3u32  = elem_32 | shape_mat2x3,
    mat2x3u64  = elem_64 | shape_mat2x3,

    mat2x3s8   =           shape_mat2x3 | type_signed,
    mat2x3s16  = elem_16 | shape_mat2x3 | type_signed,
    mat2x3s32  = elem_32 | shape_mat2x3 | type_signed,
    mat2x3s64  = elem_64 | shape_mat2x3 | type_signed,

    mat2x3fp16 = elem_16 | shape_mat2x3 | type_fp,
    mat2x3fp32 = elem_32 | shape_mat2x3 | type_fp,
    mat2x3fp64 = elem_64 | shape_mat2x3 | type_fp,

    mat2x3b    = elem_32 | shape_mat2x3 | type_boolean,


    mat3x3u8   =           shape_mat3x3,
    mat3x3u16  = elem_16 | shape_mat3x3,
    mat3x3u32  = elem_32 | shape_mat3x3,
    mat3x3u64  = elem_64 | shape_mat3x3,

    mat3x3s8   =           shape_mat3x3 | type_signed,
    mat3x3s16  = elem_16 | shape_mat3x3 | type_signed,
    mat3x3s32  = elem_32 | shape_mat3x3 | type_signed,
    mat3x3s64  = elem_64 | shape_mat3x3 | type_signed,

    mat3x3fp16 = elem_16 | shape_mat3x3 | type_fp,
    mat3x3fp32 = elem_32 | shape_mat3x3 | type_fp,
    mat3x3fp64 = elem_64 | shape_mat3x3 | type_fp,

    mat3x3b    = elem_32 | shape_mat3x3 | type_boolean,


    mat4x3u8   =           shape_mat4x3,
    mat4x3u16  = elem_16 | shape_mat4x3,
    mat4x3u32  = elem_32 | shape_mat4x3,
    mat4x3u64  = elem_64 | shape_mat4x3,

    mat4x3s8   =           shape_mat4x3 | type_signed,
    mat4x3s16  = elem_16 | shape_mat4x3 | type_signed,
    mat4x3s32  = elem_32 | shape_mat4x3 | type_signed,
    mat4x3s64  = elem_64 | shape_mat4x3 | type_signed,

    mat4x3fp16 = elem_16 | shape_mat4x3 | type_fp,
    mat4x3fp32 = elem_32 | shape_mat4x3 | type_fp,
    mat4x3fp64 = elem_64 | shape_mat4x3 | type_fp,

    mat4x3b    = elem_32 | shape_mat4x3 | type_boolean,


    mat2x4u8   =           shape_mat2x4,
    mat2x4u16  = elem_16 | shape_mat2x4,
    mat2x4u32  = elem_32 | shape_mat2x4,
    mat2x4u64  = elem_64 | shape_mat2x4,

    mat2x4s8   =           shape_mat2x4 | type_signed,
    mat2x4s16  = elem_16 | shape_mat2x4 | type_signed,
    mat2x4s32  = elem_32 | shape_mat2x4 | type_signed,
    mat2x4s64  = elem_64 | shape_mat2x4 | type_signed,

    mat2x4fp16 = elem_16 | shape_mat2x4 | type_fp,
    mat2x4fp32 = elem_32 | shape_mat2x4 | type_fp,
    mat2x4fp64 = elem_64 | shape_mat2x4 | type_fp,

    mat2x4b    = elem_32 | shape_mat2x4 | type_boolean,


    mat3x4u8   =           shape_mat3x4,
    mat3x4u16  = elem_16 | shape_mat3x4,
    mat3x4u32  = elem_32 | shape_mat3x4,
    mat3x4u64  = elem_64 | shape_mat3x4,

    mat3x4s8   =           shape_mat3x4 | type_signed,
    mat3x4s16  = elem_16 | shape_mat3x4 | type_signed,
    mat3x4s32  = elem_32 | shape_mat3x4 | type_signed,
    mat3x4s64  = elem_64 | shape_mat3x4 | type_signed,

    mat3x4fp16 = elem_16 | shape_mat3x4 | type_fp,
    mat3x4fp32 = elem_32 | shape_mat3x4 | type_fp,
    mat3x4fp64 = elem_64 | shape_mat3x4 | type_fp,

    mat3x4b    = elem_32 | shape_mat3x4 | type_boolean,


    mat4x4u8   =           shape_mat4x4,
    mat4x4u16  = elem_16 | shape_mat4x4,
    mat4x4u32  = elem_32 | shape_mat4x4,
    mat4x4u64  = elem_64 | shape_mat4x4,

    mat4x4s8   =           shape_mat4x4 | type_signed,
    mat4x4s16  = elem_16 | shape_mat4x4 | type_signed,
    mat4x4s32  = elem_32 | shape_mat4x4 | type_signed,
    mat4x4s64  = elem_64 | shape_mat4x4 | type_signed,

    mat4x4fp16 = elem_16 | shape_mat4x4 | type_fp,
    mat4x4fp32 = elem_32 | shape_mat4x4 | type_fp,
    mat4x4fp64 = elem_64 | shape_mat4x4 | type_fp,

    mat4x4b    = elem_32 | shape_mat4x4 | type_boolean,
  };

  J_INLINE_GETTER constexpr data_type with_rows(data_type v, u8_t rows) noexcept {
    return static_cast<data_type>((static_cast<u8_t>(v) & ~row_mask) | ((rows - 1U) << row_shift));
  }

  J_INLINE_GETTER constexpr data_type with_cols(data_type v, u8_t cols) noexcept {
    return static_cast<data_type>((static_cast<u8_t>(v) & ~col_mask) | ((cols - 1U) << col_shift));
  }

  J_INLINE_GETTER constexpr u8_t cols(data_type v) noexcept {
    return (((u8_t)v & col_mask) >> col_shift) + 1U;
  }

  J_INLINE_GETTER constexpr u8_t rows(data_type v) noexcept {
    return (((u8_t)v & row_mask) >> row_shift) + 1U;
  }

  J_INLINE_GETTER constexpr u8_t num_elements(data_type v) noexcept {
    return cols(v) * rows(v);
  }

  J_INLINE_GETTER constexpr u8_t element_size(data_type v) noexcept {
    if (v == data_type::none) {
      return 0U;
    }
    return 1U << ((u8_t)v & element_size_mask);
  }

  J_INLINE_GETTER constexpr bool is_float(data_type v) noexcept {
    return (u8_t)v & type_fp;
  }

  J_INLINE_GETTER constexpr u8_t packed_size(data_type v) noexcept {
    return element_size(v) * num_elements(v);
  }

  /// Get 32-bit aligned size of a data type enum.
  ///
  /// \note Booleans are 32-bit values, since Vulkan handles VkBool32 values in
  ///       specialization constants etc.
  /// \note The "none" type returns zero.
  [[nodiscard]] inline constexpr u8_t num_locations(data_type v) noexcept {
    if (v == data_type::none) {
      return 0U;
    }
    const u8_t col_size = ::j::align_up(rows(v) * element_size(v), 16U);
    return (col_size * cols(v)) / 16U;
  }
}
