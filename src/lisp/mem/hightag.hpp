#pragma once

#include "exceptions/assert_lite.hpp"

namespace j::lisp::mem {
  enum class object_hightag_flag : u8_t {
    non_gc         = 0b00000001U,
    has_debug_info = 0b00000010U,
    gc_mark        = 0b00000100U,
  };

  /// Hightag for Lisp objects.
  struct object_hightag final {
    /// Flags - is not GC-managed, is marked by GC, has debug info.
    u8_t flags = 0U;

    /// Number of references from native code. E.g. if the object is part of
    /// package or function book-keeping information.
    u8_t ext_refcount = 0U;

    /// Padding
    u16_t m_unused = 0U;

    J_INLINE_GETTER constexpr bool is_gc_managed() const noexcept {
      return !(flags & (u8_t)object_hightag_flag::non_gc);
    }

    J_INLINE_GETTER constexpr bool has_debug_info() const noexcept {
      return flags & (u8_t)object_hightag_flag::has_debug_info;
    }

    J_INLINE_GETTER constexpr bool has_mark() const noexcept {
      return flags & (u8_t)object_hightag_flag::gc_mark;
    }

    J_ALWAYS_INLINE constexpr void set_gc_managed(bool is_managed) noexcept {
      flags = (flags & ~(u8_t)object_hightag_flag::non_gc) | (is_managed ? 0U : (u8_t)object_hightag_flag::non_gc);
    }

    J_ALWAYS_INLINE constexpr void set_mark(bool mark) noexcept {
      flags = (flags & ~(u8_t)object_hightag_flag::gc_mark) | (mark ? (u8_t)object_hightag_flag::gc_mark : 0U);
    }

    J_ALWAYS_INLINE constexpr void set_has_debug_info(bool has) noexcept {
      flags = (flags & ~(u8_t)object_hightag_flag::has_debug_info) | (has ? (u8_t)object_hightag_flag::has_debug_info : 0U);
    }

    J_ALWAYS_INLINE constexpr void ext_ref() noexcept {
      ++ext_refcount;
    }

    J_ALWAYS_INLINE constexpr void ext_unref() noexcept {
      --ext_refcount;
    }
  };

  /// Hightag for heap footer.
  struct footer_hightag final {
    u32_t raw = 0U;

    J_INLINE_GETTER constexpr bool follows_free_chunk() const noexcept {
      J_ASSUME(raw == 1U || raw == 0U);
      return raw == 1U;
    }

    J_INLINE_GETTER constexpr bool follows_object() const noexcept {
      J_ASSUME(raw == 1U || raw == 0U);
      return !raw;
    }

    J_ALWAYS_INLINE void set_follows_free_chunk() noexcept {
      raw = 1U;
    }

    J_ALWAYS_INLINE void set_follows_object() noexcept {
      raw = 0U;
    }
  };

  /// Hightag for unallocated memory chunks.
  struct free_chunk_hightag final {
    /// Offset in 64-bit words to the next free memory chunk.
    u32_t next_free_offset = 0U;
  };

  /// Hightag for heap sentinels.
  struct sentinel_hightag final {
    u32_t m_unused = 0U;
  };

  /// Hightag for debug info.
  struct debug_info_hightag final {
    u32_t m_unused = 0U;
  };

  /// Hightag for activation records.
  struct activation_record_hightag final {
    u32_t m_unused = 0U;
  };

  /// Hightag for heap objects.
  ///
  /// Lisp objects:
  /// ```
  /// +--------------+--------------+-------+
  /// | Zero-padding | Ext refcount | Flags |
  /// +--------------+--------------+-------+
  ///   31..    ..16   15..     ..8   7..0
  /// ```
  ///
  /// Lisp object flags:
  ///   `0000 0001` - Not GC-managed, e.g. constant data.
  ///   `0000 0010` - Has debug info preceding the header.
  ///   `0000 0100` - GC mark. Note that mark meaning swaps after each GC.
  ///
  /// Heap footer:
  ///   Least-significant bit is set if preceded by a free chunk, clear if
  ///   preceded by a lisp object.
  ///
  /// Free chunks:
  ///   Offset in 64-bit words to the next free chunk.
  ///
  /// Heap sentinels: unused
  union hightag {
    object_hightag object_hightag = { 0U, 0U, 0U };
    footer_hightag footer_hightag;
    free_chunk_hightag free_chunk_hightag;
    sentinel_hightag sentinel_hightag;
    debug_info_hightag debug_info_hightag;
  };
}
