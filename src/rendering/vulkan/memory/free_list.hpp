#pragma once

#include "hzd/types.hpp"
#include "containers/vector.hpp"

namespace j::rendering::vulkan::memory {
  /// Entry in a free list.
  struct free_list_entry final {
    u32_t offset = 0U;
    u32_t size = 0U;
    using zero_initializable_tag_t = void;
  };
}

J_DECLARE_EXTERN_NONCOPYABLE_VECTOR(j::rendering::vulkan::memory::free_list_entry);

namespace j::rendering::vulkan::memory {
  class free_list {
    using free_list_t J_NO_DEBUG_TYPE = noncopyable_vector<free_list_entry>;
    using free_list_iterator_t J_NO_DEBUG_TYPE = free_list_t::iterator;

    J_INTERNAL_LINKAGE free_list_iterator_t allocate_full_entry(free_list_iterator_t it);

    J_INTERNAL_LINKAGE free_list_iterator_t allocate_partial_entry(free_list_iterator_t it, u32_t new_offset, u32_t new_size);
  public:
    /// Free list, sorted by offset.
    free_list_t by_offset;

    /// Total size of all free items.
    u32_t free_size;
    /// Total size of the free list, including size allocated.
    u32_t total_size;

    explicit free_list(u32_t size);

    /// Allocate [size] bytes from the start of the free list entry pointed by [it].
    ///
    /// Allocation may invalidate iterators to the free list. This function returns an iterator
    /// to the next valid free list entry at the point of iteration.
    ///
    /// \returns Iterator after the current point of iteration. May return [it] e.g. if the entry
    ///          pointed to by [it] is not completely allocated and is replaced by a new entry.
    free_list_iterator_t allocate_from_start_of(free_list_iterator_t it, u32_t size);

    /// Allocate [size] bytes from the end of the free list entry pointed by [it].
    ///
    /// Allocation may invalidate iterators to the free list. This function returns an iterator
    /// to the next valid free list entry at the point of iteration.
    ///
    /// \returns Iterator after the current point of iteration. May return [it] e.g. if the entry
    ///          pointed to by [it] is not completely allocated and is replaced by a new entry.
    free_list_iterator_t allocate_from_end_of(free_list_iterator_t it, u32_t size);

    /// Free [size] bytes at [offset] to a free-list entry before it.
    ///
    /// Freeing may invalidate iterators to the free list, so a new iterator is returned.
    /// The iterator points to the first free-list entry after the region that was freed.
    ///
    /// The element pointed to by [by_offset_it] may change in size and offset. Its offset may only
    /// get smaller.
    ///
    /// \param by_offset_it Pointer to the free-list entry to free. Must be in the by-offset list.
    ///
    /// \returns Iterator pointing to the first free-list entry after the freed area.
    free_list_iterator_t free_before(free_list_iterator_t by_offset_it, u32_t offset, u32_t size);

    /// Get the size of the largest contiguous free region in the list.
    u32_t get_largest_free_block_size() const noexcept;

    void assert_valid() const;

    void dump() const;
  };

  #ifdef NDEBUG
  inline void free_list::assert_valid() const { }
  #endif
}
