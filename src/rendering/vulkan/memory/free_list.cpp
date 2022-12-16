#include "rendering/vulkan/memory/free_list.hpp"

#include "rendering/vulkan/utils.hpp"
#include "containers/vector.hpp"
#include "logging/global.hpp"

J_DEFINE_EXTERN_NONCOPYABLE_VECTOR(j::rendering::vulkan::memory::free_list_entry);

namespace j::rendering::vulkan::memory {
  free_list::free_list(u32_t size)
    : free_size(size),
      total_size(size)
  {
    J_ASSERT_NOT_NULL(size);
    by_offset.emplace_back(free_list_entry{0, size});
  }

  free_list::free_list_iterator_t free_list::allocate_full_entry(free_list_iterator_t it) {
    return by_offset.erase(it);
  }

  free_list::free_list_iterator_t free_list::allocate_partial_entry(free_list_iterator_t it, u32_t new_offset, u32_t new_size) {
    J_ASSERT_NOT_NULL(new_size);
    it->offset = new_offset, it->size = new_size;
    return it;
  }

  free_list::free_list_iterator_t free_list::allocate_from_start_of(
    free_list_iterator_t it,
    u32_t size
  ) {
    J_ASSUME(size > 0);
    J_VK_ASSERT(size <= it->size, "Tried to allocate more than was available.");
    J_VK_ASSERT(size <= free_size, "Free size does not match.");
    free_size -= size;
    const auto result = J_UNLIKELY(size == it->size)
      ? allocate_full_entry(it)
      : allocate_partial_entry(it, it->offset + size, it->size - size);
    return assert_valid(), result;
  }

  free_list::free_list_iterator_t free_list::allocate_from_end_of(
    free_list_iterator_t it,
    u32_t size
  ) {
    J_ASSERT_NOT_NULL(size);
    J_VK_ASSERT(size <= it->size, "Tried to allocate more than was available.");
    J_VK_ASSERT(size <= free_size, "Free size does not match.");
    free_size -= size;
    const auto result = J_UNLIKELY(size == it->size)
      ? allocate_full_entry(it)
      : allocate_partial_entry(it, it->offset, it->size - size);
    return assert_valid(), result;
  }

  free_list::free_list_iterator_t free_list::free_before(free_list_iterator_t by_offset_it, u32_t offset, u32_t size) {
    J_ASSERT_NOT_NULL(size);
    J_VK_ASSERT(offset < total_size, "Offset to be freed is past free-list bounds.");
    const u32_t end_offset = size + offset;
    J_VK_ASSERT(end_offset <= total_size, "Tried to free past free-list bounds.");
    J_VK_ASSERT(by_offset_it == by_offset.end() ||
                end_offset <= by_offset_it->offset,
                "Offset of the next free entry is below the offset to be freed.");
    J_VK_ASSERT(by_offset_it == by_offset.begin() ||
                (by_offset_it - 1)->offset + (by_offset_it - 1)->size <= offset,
                "Offset of the previous free entry is above the offset to be freed.");

    free_size += size;

    const bool merges_with_next = by_offset_it != by_offset.end()
      && by_offset_it->offset == end_offset;

    if (merges_with_next) {
      size += by_offset_it->size;
    }

    const bool merges_with_previous = by_offset_it != by_offset.begin()
      && (by_offset_it - 1)->offset + (by_offset_it - 1)->size == offset;

    free_list_iterator_t previous_it = nullptr;
    if (merges_with_previous) {
      previous_it = by_offset_it - 1;
      size += previous_it->size,
      offset = previous_it->offset;
    }

    if (merges_with_next && merges_with_previous) {
      // An element will be removed. Remove the next entry,
      // making the previous entry the merged entry.
      previous_it->offset = offset, previous_it->size = size;
      by_offset_it = by_offset.erase(by_offset_it);
      return assert_valid(), previous_it + 1;
    } else if (!merges_with_previous && !merges_with_next) {
      // An element will be inserted.
      by_offset_it = by_offset.emplace(by_offset_it, free_list_entry{offset, size});
      return assert_valid(), by_offset_it;
    } else if (merges_with_previous) {
      previous_it->size = size, previous_it->size = size;
      return assert_valid(), by_offset_it;
    } else {
      by_offset_it->offset = offset, by_offset_it->size = size;
      return assert_valid(), by_offset_it + 1;
    }
  }

  u32_t free_list::get_largest_free_block_size() const noexcept {
    u32_t sz = 0;
    for (auto & e : by_offset) {
      sz = max(sz, e.size);
    }
    return sz;
  }

  #ifndef NDEBUG

  void free_list::assert_valid() const {
    try {
      J_VK_ASSERT(free_size <= total_size, "Free size is larger than the total size.");
      u32_t current_end = 0;
      u32_t current_index = 0;
      u32_t sz = 0;
      for (auto & entry : by_offset) {
        J_ASSERT_NOT_NULL(entry.size);
        J_VK_ASSERT(entry.offset >= current_end, "Overlapping entries in by_offset.");
        if (current_end) {
          J_VK_ASSERT(entry.offset != current_end, "Unmerged entries in by_offset.");
        }
        J_VK_ASSERT(entry.offset + entry.size <= total_size, "Entry size is out of bounds.");

        current_end = entry.offset + entry.size, sz += entry.size, current_index++;
      }
      J_VK_ASSERT(sz == free_size, "Free size does not match.");
    } catch (...) {
      dump();
      throw;
    }
  }
  #endif

  void free_list::dump() const {
    J_DEBUG(" {#bold,bright_green_bg,white}  Free list {}/{} bytes  {/}", free_size, total_size);
    J_DEBUG("   {#bold,bright_magenta}By offset:{/}");
    int i = 0;
    for (auto & entry : by_offset) {
      J_DEBUG("   {#bold}Entry #{}:{/}  {#bright_green}{}{/}-{#bright_yellow}{}{/} (size: {#bold}{}{/}, other: {#bold}{}{/})",
              i++, entry.offset, entry.offset + entry.size, entry.size);
    }
  }
}
