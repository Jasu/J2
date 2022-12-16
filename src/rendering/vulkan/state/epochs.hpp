#pragma once

#include "exceptions/assert_lite.hpp"

#define J_ASSERT_EPOCH_RANGE(R) J_ASSERT((R) < ::j::rendering::vulkan::state::max_epoch_types_v, \
                                         "Epoch index was out of range.")

namespace j::rendering::vulkan::state {
  /// Maximum number of epoch types in the state DAG.
  inline constexpr u32_t max_epoch_types_v = 4U;

  /// Epoch definition for State DAG (state tracker).
  ///
  /// Epochs are bitmasks with varying widths, so the definitions have to be manually specified.
  struct epoch_definition final {
    u32_t level_mask_inv = 0;
    u8_t level_shift = 0;
    u8_t index = 0U;
    const char * name = nullptr;

    constexpr epoch_definition() noexcept = default;

    constexpr epoch_definition(u8_t index, u8_t level_shift_bits, u8_t level_width_bits, const char * name) noexcept
      : level_mask_inv(~(((1U << level_width_bits) - 1U) << level_shift_bits)),
        level_shift(level_shift_bits),
        index(index),
        name(name)
    {
      J_ASSERT_EPOCH_RANGE(index);
      J_ASSERT(level_shift > 10U && level_shift <= 29U, "Level shift out of range.");
      J_ASSERT(level_width_bits > 2U && (level_shift + level_width_bits <= 32U), "Level width/mask out of range.");
      J_ASSERT_NOT_NULL(name, level_mask_inv);
    }
  };

  namespace detail { enum class end_tag { v }; }
  J_NO_DEBUG inline constexpr detail::end_tag end_tag{detail::end_tag::v};

  class epoch_key final {
  public:
    J_ALWAYS_INLINE constexpr epoch_key() noexcept = default;

    J_ALWAYS_INLINE constexpr epoch_key(detail::end_tag, u8_t type) noexcept
      : m_value(0xFFF0U | type)
    { J_ASSERT_EPOCH_RANGE(type); }

    J_ALWAYS_INLINE constexpr epoch_key(u8_t type, u16_t counter) noexcept
      : m_value((counter << 4U) | type)
    {
      J_ASSERT_EPOCH_RANGE(type);
      J_ASSERT(counter < 0xFFFU, "Counter out of range");
    }

    J_INLINE_GETTER constexpr bool is_begin() const noexcept { return m_value < 0x10U; }

    J_INLINE_GETTER constexpr bool is_end() const noexcept { return m_value >= 0xFFF0U; }

    J_INLINE_GETTER constexpr u8_t type() const noexcept { return m_value & 0xFU; }

    J_INLINE_GETTER constexpr u16_t counter() const noexcept { return m_value >> 4U; }

    J_INLINE_GETTER constexpr bool operator==(const epoch_key & rhs) const noexcept = default;

    J_INLINE_GETTER constexpr bool operator<(const epoch_key & rhs) const noexcept {
      return m_value < rhs.m_value;
    }
  private:
    u16_t m_value = 0U;
  };
}
