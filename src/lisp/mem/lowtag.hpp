#pragma once

#include "exceptions/assert_lite.hpp"

namespace j::lisp::mem {
  namespace detail {
    enum class vec_tag_t : u8_t { v };
    enum class str_tag_t : u8_t { v };
    enum class act_record_tag_t : u8_t { v };
    enum class sentinel_tag_t : u8_t { v };
    enum class footer_tag_t : u8_t { v };
    enum class free_chunk_tag_t : u8_t { v };
    enum class debug_info_tag_t : u8_t { v };
  }
  constexpr inline auto vec_tag = detail::vec_tag_t::v;
  constexpr inline auto str_tag = detail::str_tag_t::v;
  constexpr inline auto act_record_tag = detail::act_record_tag_t::v;
  constexpr inline auto sentinel_tag = detail::sentinel_tag_t::v;
  constexpr inline auto footer_tag = detail::footer_tag_t::v;
  constexpr inline auto free_chunk_tag = detail::free_chunk_tag_t::v;
  constexpr inline auto debug_info_tag = detail::debug_info_tag_t::v;

  inline constexpr u8_t lowtag_type_width_v = 4U;
  inline constexpr u8_t lowtag_type_shift_v = 32U - lowtag_type_width_v;
  inline constexpr u32_t lowtag_type_mask_v = -(1U << lowtag_type_shift_v);
  inline constexpr u32_t lowtag_is_ctrl_flag_v = 1U << 31;
  inline constexpr u32_t lowtag_data_mask_v = ~lowtag_type_mask_v;

  /// Four-bit type.
  ///
  /// MSb is zero for Lisp objects - this enables using the `MOV m64, imm32`
  /// encoding, otherwise immediate would sign-extend into high-tag.
  enum class lowtag_type : u32_t {
    vec        = 0b0000U << lowtag_type_shift_v,
    str        = 0b0001U << lowtag_type_shift_v,
    act_record = 0b0010U << lowtag_type_shift_v,

    free_chunk = 0b1000U << lowtag_type_shift_v,
    sentinel   = 0b1010U << lowtag_type_shift_v,
    footer     = 0b1100U << lowtag_type_shift_v,
    debug_info = 0b1111U << lowtag_type_shift_v,
  };

  /// Low (least-significant) tag of heap objects.
  ///
  /// ```
  /// +------+------------------+
  /// | Type |       Data       |
  /// +------+------------------+
  /// 31..28   27..           ..0
  ///
  ///       +--------+---------+
  /// Type: | IsCtrl | Subtype |
  ///       +--------+---------+
  ///       Bit 31    30..28
  /// ```
  ///
  /// The top bit (`IsCtrl`) is set for heap control values and clear for Lisp
  /// objects.
  ///
  /// Control value types:
  ///   `1 000` - Free chunk header
  ///             Data is the size of the chunk in 64-bit words, machine encoded.
  ///             E.g. `0b1000U << 28 | 171U` would be followed by 171 * 8 bytes
  ///             of free space, and a heap footer.
  ///
  ///   `1 010` - Heap arena sentinel
  ///             Marks the start or the end of the current heap arena. Data is
  ///             0 for arena begin, and 1 for arena end.
  ///
  ///   `1 100` - Region footer
  ///             Marks the end of an object or free region on heap. Data is size
  ///             of the preceding object in 64-bit words + 1, machine encoded.
  ///             E.g. `0b1100U << 28 | 12U` would mean that the footer is preceded
  ///             by an object or a free chunk of 11 * 8 bytes and its header.
  ///
  ///   `1 111` - Debug info
  ///             Provide debug information for the vector following the value.
  ///             May be repeated, e.g. for macro expansion.
  ///             Data is the type of the debug info record. E.g. `0` means
  ///             `source_location`. The size is determined by the type.
  ///
  /// Lisp object types:
  ///
  ///   `0 000` - Vec
  ///             Data is size in immediates, i.e. 64-bit words, machine encoded.
  ///             E.g. `0b0000U << 28 | 7U` would be a 7-element vector, taking
  ///             8 bytes for the header + 7 * 8 bytes for data, followed by a
  ///             heap region footer.
  ///
  ///   `0 001` - String
  ///             Data is size in bytes, machine encoded.
  ///             E.g. `0b0001U << 28 | 18U` would be a 18-byte string, taking
  ///             8 bytes for the header + 18 bytes aligned to 8 bytes, followed
  ///             by a heap region footer.
  ///
  ///   `0 010` - Activation record
  ///             A lambda activation record. Data is the size in 64-bit words.
  struct lowtag final {
    u32_t raw = 0U;

    J_BOILERPLATE(lowtag, CTOR_CE)

    J_ALWAYS_INLINE explicit constexpr lowtag(lowtag_type type, u32_t data = 0U) noexcept
      : raw((u32_t)type | data)
    {
      J_ASSUME(data <= lowtag_data_mask_v);
    }

    J_INLINE_GETTER_NO_DEBUG constexpr u32_t data() const noexcept {
      return raw & lowtag_data_mask_v;
    }

    J_INLINE_GETTER_NO_DEBUG constexpr lowtag_type type() const noexcept {
      return (lowtag_type)(lowtag_type_mask_v & raw);
    }

    J_ALWAYS_INLINE constexpr void set_data(u32_t data) noexcept {
      J_ASSUME(data <= lowtag_data_mask_v);
      raw = (raw & lowtag_type_mask_v) | data;
    }

    J_INLINE_GETTER constexpr bool is_object() const noexcept {
      return !(raw & lowtag_is_ctrl_flag_v);
    }

    J_INLINE_GETTER constexpr bool is_ctrl() const noexcept {
      return raw & lowtag_is_ctrl_flag_v;
    }
  };
}
