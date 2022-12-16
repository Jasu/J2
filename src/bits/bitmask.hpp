#pragma once

#include "bits/bitmask_common.hpp"
#include "bits/bitops.hpp"
#include "bits/bitmask_iterator_without_offset.hpp"
#include "containers/pair.hpp"
#include "hzd/type_traits.hpp"

namespace j::bits {
  namespace detail {
    /// Note that u16_t is skipped, since on x86, operations on 16-bit fields are dog slow.
    template<auto MaxOffset, typename Enable = void>
    struct fitting_array {
      inline constexpr static const u8_t shift = 6;
      inline constexpr static const u8_t mask = 63;
      inline constexpr static const u32_t num_elems = (MaxOffset + mask + 1) >> shift;
      using element_type J_NO_DEBUG_TYPE = u64_t;
      using type J_NO_DEBUG_TYPE = element_type[num_elems];
    };

    template<auto MaxOffset>
    struct fitting_array<MaxOffset, j::enable_if_t<MaxOffset < 32 && 7 < MaxOffset>> {
      inline constexpr static const u8_t shift = 5;
      inline constexpr static const u8_t mask = 31;
      inline constexpr static const u32_t num_elems = (MaxOffset + mask + 1) >> shift;
      using element_type J_NO_DEBUG_TYPE = u32_t;
      using type J_NO_DEBUG_TYPE = element_type[num_elems];
    };

    template<auto MaxOffset>
    struct fitting_array<MaxOffset, j::enable_if_t<MaxOffset < 8>> {
      inline constexpr static const u8_t shift = 3;
      inline constexpr static const u8_t mask = 7;
      inline constexpr static const u32_t num_elems = (MaxOffset + mask + 1) >> shift;
      using element_type J_NO_DEBUG_TYPE = u8_t;
      using type J_NO_DEBUG_TYPE = element_type[num_elems];
    };
  }

  template<typename Enum, Enum MaxValue>
  class bitmask {
    static_assert(static_cast<u64_t>(MaxValue) < 255);

    using array_info_t J_NO_DEBUG_TYPE = detail::fitting_array<static_cast<u64_t>(MaxValue)>;
    using element_t J_NO_DEBUG_TYPE = typename array_info_t::element_type;
    inline constexpr static const u8_t shift = array_info_t::shift;
    inline constexpr static const u8_t mask = array_info_t::mask;
    inline constexpr static const u32_t num_elems = array_info_t::num_elems;
    inline constexpr static const u8_t end_offset = static_cast<u8_t>(MaxValue) + 1;

    element_t m_data[array_info_t::num_elems];

    constexpr void set(Enum e) noexcept {
      m_data[static_cast<u8_t>(e) >> shift] |= 1ULL << (static_cast<u8_t>(e) & mask);
    }
  public:
    using key_type J_NO_DEBUG_TYPE = Enum;
    using value_type J_NO_DEBUG_TYPE = Enum;
    bitmask(detail::for_overwrite_tag) noexcept { }

    constexpr bitmask() noexcept : m_data{ 0 } { }

    template<typename... Values>
    J_ALWAYS_INLINE constexpr explicit bitmask(Enum e, Values ... v) noexcept : m_data { 0 } {
      set(e);
      (set(v), ...);
    }

    using const_iterator J_NO_DEBUG_TYPE = bitmask_iterator_without_offset<element_t, Enum>;
    using iterator J_NO_DEBUG_TYPE = const_iterator;

    J_INLINE_GETTER const_iterator begin() const noexcept {
      return const_iterator(m_data, 0, end_offset);
    }

    J_INLINE_GETTER const_iterator end() const noexcept {
      return const_iterator(end_offset);
    }

    J_INLINE_GETTER bool contains(Enum value) const noexcept {
      u32_t offset = static_cast<u32_t>(value);
      return m_data[offset >> shift] & (1ULL << (offset & mask));
    }

    J_INLINE_GETTER const_iterator find(Enum value) const noexcept {
      if (!contains(value)) {
        return end();
      }
      const u32_t offset = static_cast<u32_t>(value);
      return const_iterator(&m_data[offset >> shift], offset, end_offset);
    }

    J_INLINE_GETTER bool empty() const noexcept {
      for (auto & d : m_data) {
        if (d) {
          return false;
        }
      }
      return true;
    }

    u8_t size() const noexcept {
      u8_t result = 0;
      for (auto & d : m_data) {
        result += popcount(d);
      }
      return result;
    }

    J_ALWAYS_INLINE constexpr void clear() noexcept {
      for (auto & d : m_data) {
        d = 0;
      }
    }

    pair<iterator, bool> insert(Enum value) noexcept {
      u8_t offset = static_cast<u8_t>(value);
      auto ptr = &m_data[offset >> shift];
      bool contains = *ptr & (1ULL << (offset & mask));
      *ptr |= 1ULL << (offset & mask);
      return {iterator(ptr, offset, end_offset), !contains};
    }

    pair<iterator, bool> emplace(Enum value) noexcept {
      return insert(value);
    }

    u8_t erase(Enum value) noexcept {
      u8_t offset = static_cast<u8_t>(value);
      auto ptr = &m_data[offset >> shift];
      bool contains = *ptr & (1ULL << (offset & mask));
      *ptr &= ~(1ULL << (offset & mask));
      return contains;
    }

    iterator erase(const_iterator it) noexcept {
      *const_cast<element_t*>(it.ptr()) &= ~(1ULL << (it.offset() & mask));
      return ++it;
    }

    bitmask & operator|=(const bitmask & rhs) noexcept {
      for (u32_t i = 0; i < num_elems; ++i) {
        m_data[i] |= rhs.m_data[i];
      }
      return *this;
    }

    bitmask & operator&=(const bitmask & rhs) noexcept {
      for (u32_t i = 0; i < num_elems; ++i) {
        m_data[i] &= rhs.m_data[i];
      }
      return *this;
    }

    bitmask & operator^=(const bitmask & rhs) noexcept {
      for (u32_t i = 0; i < num_elems; ++i) {
        m_data[i] ^= rhs.m_data[i];
      }
      return *this;
    }

    bitmask operator|(const bitmask & rhs) const noexcept {
      bitmask result(for_overwrite_tag);
      for (u32_t i = 0; i < num_elems; ++i) {
        result.m_data[i] = m_data[i] | rhs.m_data[i];
      }
      return result;
    }

    bitmask operator&(const bitmask & rhs) const noexcept {
      bitmask result(for_overwrite_tag);
      for (u32_t i = 0; i < num_elems; ++i) {
        result.m_data[i] = m_data[i] & rhs.m_data[i];
      }
      return result;
    }

    bitmask operator^(const bitmask & rhs) const noexcept {
      bitmask result(for_overwrite_tag);
      for (u32_t i = 0; i < num_elems; ++i) {
        result.m_data[i] = m_data[i] ^ rhs.m_data[i];
      }
      return result;
    }

    bool operator==(const bitmask & rhs) const noexcept {
      for (u32_t i = 0; i < num_elems; ++i) {
        if (m_data[i] != rhs.m_data[i]) {
          return false;
        }
      }
      return true;
    }

    bool operator!=(const bitmask & rhs) const noexcept {
      return !(*this == rhs);
    }
  };
}
