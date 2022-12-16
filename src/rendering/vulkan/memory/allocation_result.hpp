#pragma once

#include "hzd/utility.hpp"

namespace j::rendering::vulkan::memory {
  /// Description of a result of a single allocation.
  class allocation_result {
  public:
    inline constexpr static u32_t invalid_offset_v = U32_MAX;

    /// Construct a successful allocation result.
    J_ALWAYS_INLINE explicit allocation_result(u32_t offset)
      : m_offset(offset)
    {
      assert_successful();
    }

    /// Construct an unsuccessful allocation result.
    constexpr allocation_result() noexcept = default;

    J_INLINE_GETTER bool is_successful() const noexcept {
      return J_LIKELY(m_offset != invalid_offset_v);
    }

    J_INLINE_GETTER explicit operator bool() const noexcept {
      return J_UNLIKELY(m_offset != invalid_offset_v);
    }

    J_INLINE_GETTER bool operator!() const noexcept {
      return J_UNLIKELY(m_offset == invalid_offset_v);
    }

    void assert_successful() const {
      if (J_UNLIKELY(m_offset == invalid_offset_v)) {
        throw_unsuccessful();
      }
    }

    /// Return the offset of the allocated region.
    ///
    /// Throw if the allocatoin was not successful.
    u32_t offset() const {
      assert_successful();
      return m_offset;
    }
  private:
    /// Offset of the allocated region.
    ///
    /// If the allocation is invalid, [m_offset] is [invalid_offset_v].
    u32_t m_offset = invalid_offset_v;

    [[noreturn]] static void throw_unsuccessful();
  };
}
