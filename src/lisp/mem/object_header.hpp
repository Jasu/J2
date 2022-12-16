#pragma once

#include "hzd/utility.hpp"
#include "lisp/mem/lowtag.hpp"
#include "lisp/mem/hightag.hpp"
#include "lisp/mem/debug_info.hpp"

namespace j::lisp::mem {
  /// Header that starts a heap (or static) allocated Lisp object.
  struct object_header final {
    struct lowtag lowtag;
    union hightag hightag = {{0U, 0U, 0U}};

    J_ALWAYS_INLINE constexpr object_header() noexcept = default;

    J_A(AI,ND) inline constexpr object_header(
      detail::vec_tag_t,
      u32_t size_qwords,
      object_hightag_flag flags = (object_hightag_flag)0U,
      u8_t ext_refs = 0U) noexcept
      : lowtag(lowtag_type::vec, size_qwords),
        hightag{ .object_hightag = { (u8_t)flags, ext_refs, 0U } }
    { }

    J_A(AI,ND) inline constexpr object_header(
      detail::str_tag_t,
      u32_t size_bytes,
      object_hightag_flag flags = (object_hightag_flag)0U,
      u8_t ext_refs = 0U) noexcept
      : lowtag(lowtag_type::str, size_bytes),
        hightag{ .object_hightag = { (u8_t)flags, ext_refs, 0U } }
    { }

    J_A(AI,ND) inline constexpr object_header(
      detail::act_record_tag_t,
      u32_t size_qwords,
      object_hightag_flag flags = (object_hightag_flag)0U,
      u8_t ext_refs = 0U) noexcept
      : lowtag(lowtag_type::act_record, size_qwords),
        hightag{ .object_hightag = { (u8_t)flags, ext_refs, 0U } }
    { }

    J_A(AI,ND) inline constexpr object_header(detail::sentinel_tag_t, bool is_end) noexcept
      : lowtag(lowtag_type::sentinel, is_end ? 1U : 0U),
        hightag{ .sentinel_hightag = {} }
    { }

    J_A(AI,ND) inline constexpr object_header(detail::footer_tag_t, u32_t offset_qwords, bool is_free_chunk) noexcept
      : lowtag(lowtag_type::footer, offset_qwords),
        hightag{ .footer_hightag = { is_free_chunk ? 1U : 0U } }
    {
      // 0-sized object have header + footer, and footer_ptr - offset_qwords must yield the header.
      J_ASSUME(offset_qwords >= 1U);
    }

    J_A(AI,ND) inline constexpr object_header(detail::free_chunk_tag_t, u32_t size_qwords, u32_t next_offset_qwords) noexcept
      : lowtag(lowtag_type::free_chunk, size_qwords),
        hightag{ .free_chunk_hightag = { next_offset_qwords } }
    {
      // The minimum size of the free chunk is header + footer, but 0 marks the last free chunk in the arena.
      J_ASSUME(!next_offset_qwords || next_offset_qwords >= 2U);
    }

    J_A(AI,ND) inline constexpr object_header(detail::debug_info_tag_t, debug_info_type type) noexcept
      : lowtag(lowtag_type::debug_info, (u32_t)type),
        hightag{ .debug_info_hightag = { } }
    { }

    J_INLINE_GETTER constexpr lowtag_type type() const noexcept {
      return lowtag.type();
    }

    J_INLINE_GETTER constexpr u32_t lowtag_data() const noexcept {
      return lowtag.data();
    }

    J_ALWAYS_INLINE void set_lowtag_data(u32_t data) noexcept {
      return lowtag.set_data(data);
    }

    J_INLINE_GETTER struct object_hightag & object_hightag() noexcept {
      J_ASSERT(lowtag.is_object());
      return hightag.object_hightag;
    }

    J_INLINE_GETTER const struct object_hightag & object_hightag() const noexcept {
      return const_cast<object_header*>(this)->object_hightag();
    }

    J_INLINE_GETTER struct sentinel_hightag & sentinel_hightag() noexcept {
      J_ASSERT(lowtag.type() == lowtag_type::sentinel);
      return hightag.sentinel_hightag;
    }

    J_INLINE_GETTER const struct sentinel_hightag & sentinel_hightag() const noexcept {
      return const_cast<object_header*>(this)->sentinel_hightag();
    }

    J_INLINE_GETTER struct footer_hightag & footer_hightag() noexcept {
      J_ASSERT(lowtag.type() == lowtag_type::footer);
      return hightag.footer_hightag;
    }

    J_INLINE_GETTER const struct footer_hightag & footer_hightag() const noexcept {
      return const_cast<object_header*>(this)->footer_hightag();
    }

    J_INLINE_GETTER struct free_chunk_hightag & free_chunk_hightag() noexcept {
      J_ASSERT(lowtag.type() == lowtag_type::free_chunk);
      return hightag.free_chunk_hightag;
    }

    J_INLINE_GETTER const struct free_chunk_hightag & free_chunk_hightag() const noexcept {
      return const_cast<object_header*>(this)->free_chunk_hightag();
    }

    J_INLINE_GETTER struct debug_info_hightag & debug_info_hightag() noexcept {
      J_ASSERT(lowtag.type() == lowtag_type::debug_info);
      return hightag.debug_info_hightag;
    }

    J_INLINE_GETTER const struct debug_info_hightag & debug_info_hightag() const noexcept {
      return const_cast<object_header*>(this)->debug_info_hightag();
    }
  };
}
