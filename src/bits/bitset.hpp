#pragma once

#include "bits/bitops.hpp"
#include "hzd/mem.hpp"
#include "hzd/string.hpp"

namespace j::bits {
  struct set_bit_iterator final {
    const u64_t * it = nullptr;
    u32_t idx = 0U;
    u32_t qword_idx = 0U;
    u32_t max = 0U;

    set_bit_iterator & fast_forward() {
      while (qword_idx != max && !(*it & (1UL << idx))) {
        idx = ctz_safe(*it & -(1UL << idx));
        if (idx < 64U || qword_idx == max) {
          break;
        }
        qword_idx++;
        idx = 0U;
        ++it;
      }
      return *this;
    }

    J_A(ND,AI,NODISC) constexpr bool operator==(const set_bit_iterator &) const noexcept = default;

    set_bit_iterator & operator++() noexcept {
      if (++idx == 64U) {
        idx = 0U;
        ++it;
        ++qword_idx;
      }
      if (qword_idx != max) {
        fast_forward();
      }
      return *this;
    }

    set_bit_iterator operator++(int) noexcept {
      set_bit_iterator res(*this);
      operator++();
      return res;
    }

    u32_t operator*() const noexcept {
      return qword_idx * 64U + idx;
    }
  };

  class bitset {
    inline void allocate(u32_t size) {
      m_size_bits = size;
      m_size_bytes = j::align_up(size, 64U) >> 3;
      if (size > 64) {
        m_data = (u64_t*)::j::allocate_zero(m_size_bytes);
      } else {
        m_data_inline = 0U;
      }
    }

    inline void deallocate() noexcept {
      if (m_size_bytes > 8) {
        ::j::free(m_data);
      }
    }
  public:
    constexpr static inline u32_t npos = U32_MAX;

    J_ALWAYS_INLINE constexpr bitset() noexcept = default;

    inline explicit bitset(u32_t size) {
      allocate(size);
    }

    J_A(AI) constexpr bitset(bitset && rhs) noexcept
      : m_data(rhs.m_data),
        m_size_bits(rhs.m_size_bits),
        m_size_bytes(rhs.m_size_bytes)
      {
        rhs.m_data = nullptr;
        rhs.m_size_bits = 0U;
        rhs.m_size_bytes = 0U;
      }

    bitset(const bitset & rhs) {
      allocate(rhs.m_size_bits);
      if (m_size_bytes > 8) {
        ::j::memcpy(m_data, rhs.m_data, m_size_bytes);
      } else {
        m_data_inline = rhs.m_data_inline;
      }
    }

    bitset & operator=(bitset && rhs) noexcept {
      if (J_LIKELY(this != &rhs)) {
        deallocate();
        m_data = rhs.m_data;
        m_size_bytes = rhs.m_size_bytes;
        m_size_bits = rhs.m_size_bits;
        rhs.m_data = nullptr, rhs.m_size_bits = rhs.m_size_bytes = 0U;
      }
      return *this;
    }

    bitset & operator=(const bitset & rhs) {
      if (J_LIKELY(this != &rhs)) {
        if (m_size_bytes != rhs.m_size_bytes) {
          deallocate();
          allocate(rhs.m_size_bits);
        }
        if (m_size_bytes > 8) {
          ::j::memcpy(m_data, rhs.m_data, m_size_bytes);
        } else {
          m_data_inline = rhs.m_data_inline;
        }
      }
      return *this;
    }

    J_INLINE_GETTER bool operator[](u32_t pos) const noexcept {
      J_ASSUME(pos < m_size_bits);
      return data()[pos >> 6U] & (1ULL << (pos & 63ULL));
    }

    J_INLINE_GETTER bool is_set(u32_t pos) const noexcept {
      return operator[](pos);
    }

    J_ALWAYS_INLINE void set(u32_t pos) noexcept {
      J_ASSUME(pos < m_size_bits);
      data()[pos >> 6U] |= 1UL << (pos & 63U);
    }

    set_bit_iterator begin() const noexcept {
      return set_bit_iterator{data(), 0, 0, m_size_bytes / 8}.fast_forward();
    }
    set_bit_iterator end() const noexcept {
      return set_bit_iterator{data() + m_size_bytes / 8, 0, m_size_bytes / 8, m_size_bytes / 8};
    }

    bitset & operator |=(const bitset & rhs) noexcept {
      J_ASSUME(m_size_bits == rhs.m_size_bits);
      J_ASSUME(m_size_bytes == rhs.m_size_bytes);
      auto that = rhs.data();
      for (auto it = data(), end = data_end(); it != end; ++it, ++that) {
        *it |= *that;
      }
      return *this;
    }

    bitset operator |(const bitset & rhs) const noexcept {
      J_ASSUME(m_size_bits == rhs.m_size_bits);
      J_ASSUME(m_size_bytes == rhs.m_size_bytes);
      bitset result = bitset(m_size_bits);
      auto to = result.data();
      for (auto it = data(), that = rhs.data(), end = data_end(); it != end; ++it, ++that, ++to) {
        *to = *it | *that;
      }
      return result;
    }

    bitset & operator &=(const bitset & rhs) noexcept {
      J_ASSUME(m_size_bits == rhs.m_size_bits);
      J_ASSUME(m_size_bytes == rhs.m_size_bytes);
      auto that = rhs.data();
      for (auto it = data(), end = data_end(); it != end; ++it, ++that) {
        *it &= *that;
      }
      return *this;
    }

    bitset operator &(const bitset & rhs) const noexcept {
      J_ASSUME(m_size_bits == rhs.m_size_bits);
      J_ASSUME(m_size_bytes == rhs.m_size_bytes);
      bitset result = bitset(m_size_bits);
      auto to = result.data();
      for (auto it = data(), that = rhs.data(), end = data_end(); it != end; ++it, ++that, ++to) {
        *to = *it & *that;
      }
      return result;
    }

    bitset & operator -=(const bitset & rhs) noexcept {
      J_ASSUME(m_size_bits == rhs.m_size_bits && m_size_bytes == rhs.m_size_bytes);
      auto that = rhs.data();
      for (auto it = data(), end = data_end(); it != end; ++it, ++that) {
        *it &= ~*that;
      }
      return *this;
    }

    bitset operator -(const bitset & rhs) const noexcept {
      J_ASSUME(m_size_bits == rhs.m_size_bits && m_size_bytes == rhs.m_size_bytes);
      bitset result = bitset(m_size_bits);
      auto to = result.data();
      for (auto it = data(), that = rhs.data(), end = data_end(); it != end; ++it, ++that, ++to) {
        *to = *it & ~*that;
      }
      return result;
    }

    bool operator ==(const bitset & rhs) const noexcept {
      if (m_size_bits != rhs.m_size_bits) {
        return false;
      }
      J_ASSUME(m_size_bytes == rhs.m_size_bytes);
      for (auto it = data(), that = rhs.data(), end = data_end(); it != end; ++it, ++that) {
        if (*it != *that) {
          return false;
        }
      }
      return true;
    }

    void reset(u32_t pos) noexcept {
      J_ASSUME(pos < m_size_bits);
      data()[pos >> 6U] &= ~(1ULL << (pos & 63));
    }

    J_INLINE_GETTER u32_t find_first_clear(u32_t begin = 0U) const noexcept {
      return const_cast<bitset*>(this)->find_and_clear<false, false>(begin);
    }

    J_INLINE_GETTER u32_t find_and_set_first_clear(u32_t begin = 0U) noexcept {
      return find_and_clear<false, true>(begin);
    }

    J_INLINE_GETTER u32_t find_first_set(u32_t begin = 0U) const noexcept {
      return const_cast<bitset*>(this)->find_and_clear<true, false>(begin);
    }

    J_INLINE_GETTER u32_t find_and_clear_first_set(u32_t begin = 0U) noexcept {
      return find_and_clear<true, true>(begin);
    }

    [[nodiscard]] inline bool empty() const noexcept {
      if (m_size_bytes > 8) {
        for (u32_t i = 0U, max = m_size_bytes >> 3; i < max; ++i) {
          if (m_data[i]) {
            return false;
          }
        }
        return true;
      }
      return !m_data_inline;
    }

    J_ALWAYS_INLINE void clear() noexcept {
      if (m_size_bytes > 8) {
        for (u32_t i = 0U, max = m_size_bytes >> 3; i < max; ++i) {
          m_data[i] = 0U;
        }
      } else {
        m_data_inline = 0U;
      }
    }

    u32_t size() const noexcept {
      if (m_size_bytes > 8) {
        u32_t result = 0U;
        for (u32_t i = 0U, max = m_size_bytes >> 3; i < max; ++i) {
          result += popcount(m_data[i]);
        }
        return result;
      }
      return popcount(m_data_inline);
    }

    J_INLINE_GETTER u32_t capacity() const noexcept {
      return m_size_bits;
    }

    J_ALWAYS_INLINE ~bitset() {
      deallocate();
    }
  private:
    union {
      u64_t * m_data = nullptr;
      u64_t m_data_inline;
    };
    u32_t m_size_bits = 0U;
    u32_t m_size_bytes = 0U;

    J_A(AI,ND,RNN) u64_t * data() noexcept {
      return m_size_bytes > 8 ? m_data : &m_data_inline;
    }

    J_A(AI,ND,RNN) const u64_t * data() const noexcept {
      return m_size_bytes > 8 ? m_data : &m_data_inline;
    }
    J_A(AI,ND,RNN) u64_t * data_end() noexcept {
      return m_size_bytes > 8 ? m_data + (m_size_bytes >> 3) : (&m_data_inline + 1);
    }

    J_A(AI,ND,RNN) const u64_t * data_end() const noexcept {
      return m_size_bytes > 8 ? m_data + (m_size_bytes >> 3) : (&m_data_inline + 1);
    }

    template<bool FindSet>
    J_INLINE_GETTER static i32_t find_bit(u64_t word) noexcept {
      if constexpr (FindSet) {
        return __builtin_ffsll(word);
      } else {
        return __builtin_ffsll(~word);
      }
    }

    template<bool FindSet, bool Toggle>
    J_INLINE_GETTER u32_t find_and_clear(u32_t begin) noexcept {
      auto d = data();
      J_ASSERT(begin < m_size_bits, "Begin out of range");
      u32_t i = begin >> 6U;
      int pos;
      if ((pos = find_bit<FindSet>(d[i] >> (begin & 63U)))) {
        pos -= 1 - (begin & 63U);
        goto end;
      }
      while (++i != ((m_size_bytes + sizeof(u64_t) - 1) / sizeof(u64_t))) {
        if constexpr (FindSet) {
          if (!d[i]) { continue; }
        } else {
          if (d[i] == U64_MAX) { continue; }
        }
        pos = find_bit<FindSet>(d[i]) - 1;
        goto end;
      }
      return npos;
    end:
      const u32_t result = (i << 6U) + pos;
      if (result >= m_size_bits) {
        return npos;
      }
      if constexpr (Toggle) {
        if constexpr (FindSet) {
          d[i] -= 1ULL << pos;
        } else {
          d[i] += 1ULL << pos;
        }
      }
      return result;
    }
  };
}
