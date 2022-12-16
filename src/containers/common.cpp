#include "containers/common.hpp"
#include "exceptions/exceptions.hpp"
#include "strings/string.hpp"
#include "containers/unsorted_string_map.hpp"

#include "hzd/string.hpp"
#include "hzd/mem.hpp"

namespace j::detail {
  J_A(NODESTROY) const strings::string string_sentinel{strings::not_a_string_tag, 0U};

  void * allocate_and_copy(u32_t bytes, const void * from, u32_t copy_size) {
    J_ASSERT_NOT_NULL(bytes, from, copy_size);
    void * result = ::j::allocate(bytes);
    ::j::memcpy(result, from, copy_size);
    return result;
  }

  void free_strings(strings::string * strings, u32_t size) noexcept {
    while (size--) {
      strings[size].~string();
    }
    ::j::free(strings);
  }

  strings::string * allocate_strings(u32_t size, u32_t value_size) {
    J_ASSERT_NOT_NULL(size, value_size);
    strings::string * result = reinterpret_cast<strings::string *>(::j::allocate((size + 1U) * sizeof(strings::string) + size * value_size));
    for (u32_t i = 0; i <= size; ++i) {
      ::new (result + i) strings::string(strings::not_a_string_tag, 0U);
    }
    return result;
  }

  strings::string * reallocate_strings(strings::string * ptr, u32_t old_size, u32_t new_size, u32_t value_size) {
    J_ASSERT_NOT_NULL(ptr, old_size, new_size, value_size);
    strings::string * result = reinterpret_cast<strings::string *>(::j::reallocate(ptr, (new_size + 1U) * sizeof(strings::string) + new_size * value_size));
    ::j::memmove(result + new_size + 1, result + old_size + 1, old_size * value_size);
    for (u32_t i = old_size + 1; i <= new_size; ++i) {
      ::new (result + i) strings::string(strings::not_a_string_tag, 0U);
    }
    return result;
  }

  strings::string * allocate_and_copy_strings(const strings::string * ptr, u32_t old_size, u32_t new_size, u32_t value_size) {
    J_ASSERT_NOT_NULL(ptr, old_size, new_size, value_size);
    strings::string * result = reinterpret_cast<strings::string *>(::j::allocate((new_size + 1U) * sizeof(strings::string) + new_size * value_size));
    ::j::memcpy(result, ptr, min(old_size, new_size) * sizeof(strings::string));
    for (u32_t i = old_size; i <= new_size; ++i) {
      ::new (result + i) strings::string(strings::not_a_string_tag, 0U);
    }
    return result;
  }
}
