#pragma once

#include "util/flags.hpp"

namespace j::mem::vmem {
  /// Memory protection flags of a page range.
  ///
  /// The values map to those used by mmap in Unixes.
  enum J_A(FLAGS) protection_flags : u8_t {
    none = 0b000,

    r    = 0b001,
    w    = 0b010,
    x    = 0b100,

    rw   = 0b011,
    rx   = 0b101,
    rwx  = 0b111U,
  };


  /// Tag value for disallowing moving the page range when growing it.
  ///
  /// Corresponds to the absence of the MREMAP_MAYMOVE flag given to mremap.
  enum allow_move_t { allow_move };
  enum disallow_move_t { disallow_move };

  /// Memory allocation flags of a page range.
  enum J_A(FLAGS) alloc_flags : u8_t {
    normal = 0b0,
    /// Do not reserve memory for the allocation.
    ///
    /// This allows mapping gigabytes of memory, while only paying for what is used.
    no_reserve = 0b1,
  };

  /// Map anonymous pages.
  [[nodiscard]] J_RETURNS_NONNULL void * map(u32_t bytes, alloc_flags flags,
                                             protection_flags prot = protection_flags::rw);

  /// Map anonymous pages.
  [[nodiscard]] J_RETURNS_NONNULL void * map(u32_t bytes);

  /// Unmap pages.
  void unmap(void * J_NOT_NULL page, u32_t bytes);

  /// Change the protection flags of some memory pages.
  void protect(void * J_NOT_NULL ptr, u32_t bytes, protection_flags flags);

  /// Grow a memory mapping, possibly moving it.
  [[nodiscard]] void * grow(void * J_NOT_NULL ptr, u32_t old_size, u32_t new_size, bool allow_move);

  /// Grow a memory mapping, keeping it at the same address.
  ///
  /// \return ptr if successful, null if not.
  J_A(HIDDEN,AI,NODISC,RNN,ND) inline void * grow(disallow_move_t, void * J_NOT_NULL ptr, u32_t old_size, u32_t new_size) {
    return grow(ptr, old_size, new_size, false);
  }

  /// Grow a memory mapping, maybe moving it to a different virtual address.
  ///
  /// \return ptr, or its new location.
  J_A(HIDDEN,AI,NODISC,RNN,ND) inline void * grow(allow_move_t, void * J_NOT_NULL ptr, u32_t old_size, u32_t new_size) {
    return grow(ptr, old_size, new_size, true);
  }


  /// Replace page at to_ptr with pages from from_ptr.
  void replace_pages(void * J_NOT_NULL to_ptr, void * J_NOT_NULL from_ptr, u32_t bytes);
}
