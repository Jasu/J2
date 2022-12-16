#pragma once

#include "exceptions/assert_lite.hpp"
#include "hzd/mem.hpp"

namespace j::mem {
  /// A byte range in memory.
  template<typename Ptr>
  class basic_memory_region {
  public:
    using const_ptr = const remove_ptr_t<Ptr> *;
    using void_ptr = conditional_t<is_const_v<remove_ptr_t<Ptr>>, const void *, void *>;

    /// Construct an empty memory region.
    J_ALWAYS_INLINE constexpr basic_memory_region() noexcept = default;

    /// Construct a memory region at begin with the size given.
    J_ALWAYS_INLINE constexpr basic_memory_region(void_ptr begin, u32_t size) noexcept
      : m_begin((Ptr)begin),
        m_size(size)
    { }

    /// Get the start of the memory region.
    J_INLINE_GETTER constexpr Ptr begin() noexcept { return m_begin; }
    /// Get the start of the memory region.
    J_INLINE_GETTER constexpr const_ptr begin() const noexcept { return m_begin; }

    /// Get the end of the memory region.
    [[nodiscard]]inline constexpr Ptr end() noexcept { return add_bytes(m_begin, m_size); }
    /// Get the end of the memory region.
    [[nodiscard]] inline constexpr const_ptr end() const noexcept { return add_bytes(m_begin, m_size); }

    /// Get the size of the region.
    J_INLINE_GETTER constexpr u32_t size() const noexcept { return m_size; }

    /// Return pointer to the region at the byte offset given.
    [[nodiscard]] Ptr ptr_at(u32_t byte_offset) noexcept {
      J_ASSUME(byte_offset < m_size);
      J_ASSUME_NOT_NULL(m_begin);
      return add_bytes(m_begin, byte_offset);
    }

    /// Return pointer to the region at the byte offset given.
    [[nodiscard]] inline const_ptr ptr_at(u32_t byte_offset) const noexcept {
      return const_cast<basic_memory_region*>(this)->ptr_at(byte_offset);
    }

    [[nodiscard]] basic_memory_region<const_ptr> prefix(u32_t sz) const noexcept {
      assert_size_range(sz);
      return basic_memory_region<const_ptr>(m_begin, sz);
    }

    [[nodiscard]] basic_memory_region<Ptr> prefix(u32_t sz) noexcept {
      assert_size_range(sz);
      return basic_memory_region<Ptr>(m_begin, sz);
    }

    [[nodiscard]] basic_memory_region<const_ptr> suffix(u32_t sz) const noexcept {
      assert_size_range(sz);
      return basic_memory_region<const_ptr>(add_bytes(m_begin, m_size - sz), sz);
    }

    [[nodiscard]] basic_memory_region<Ptr> suffix(u32_t sz) noexcept {
      assert_size_range(sz);
      return basic_memory_region<Ptr>(add_bytes(m_begin, m_size - sz), sz);
    }

    basic_memory_region take_prefix(u32_t sz) noexcept {
      assert_size_range(sz);
      auto result = basic_memory_region(m_begin, sz);
      m_begin = add_bytes(m_begin, sz);
      m_size -= sz;
      return result;
    }

    basic_memory_region take_suffix(u32_t sz) noexcept {
      assert_size_range(sz);
      m_size -= sz;
      return basic_memory_region(add_bytes(m_begin, m_size), sz);
    }

    [[nodiscard]] basic_memory_region without_prefix(u32_t sz) noexcept {
      assert_size_range(sz);
      return basic_memory_region(add_bytes(m_begin, sz), m_size - sz);
    }

    [[nodiscard]] basic_memory_region without_suffix(u32_t sz) noexcept {
      assert_size_range(sz);
      return basic_memory_region(m_begin, m_size - sz);
    }

    template<typename C>
    [[nodiscard]] bool intersects_with(const basic_memory_region<C> & rhs) const noexcept {
      return (const char *)m_begin < (const char *)rhs.end() && (const char *)end() > (const char *)rhs.begin();
    }

    template<typename C>
    [[nodiscard]] bool contains(const basic_memory_region<C> & rhs) const noexcept {
      return m_begin <= rhs.begin() && end() >= rhs.end();
    }

    [[nodiscard]] bool contains(const void * ptr) const noexcept {
      return m_begin <= ptr && add_bytes(m_begin, m_size) > ptr;
    }

    template<typename To>
    J_ALWAYS_INLINE void copy_to(To * J_NOT_NULL J_RESTRICT to) const noexcept {
      J_ASSUME_NOT_NULL(m_begin);
      J_ASSUME(m_size > 0U);
      ::j::memcpy(to, m_begin, m_size);
    }

    template<typename T>
    J_ALWAYS_INLINE void copy_to(basic_memory_region<T> & other) const noexcept {
      J_ASSUME_NOT_NULL(m_begin);
      J_ASSERT(m_size >= other.size());
      ::j::memcpy(other.begin(), m_begin, m_size);
    }

    J_INLINE_GETTER constexpr bool empty() const noexcept             { return !m_size; }
    J_INLINE_GETTER constexpr bool operator!() const noexcept         { return !m_size; }
    J_INLINE_GETTER constexpr explicit operator bool() const noexcept { return m_size; }

    inline void reset() noexcept {
      m_begin = nullptr;
      m_size = 0U;
    }

    /// Check that the memory regions point to the same memory region.
    J_INLINE_GETTER constexpr bool operator==(const basic_memory_region & rhs) const noexcept = default;

    J_INLINE_GETTER constexpr operator basic_memory_region<const_ptr>() const noexcept
    { return basic_memory_region<const_ptr>(m_begin, m_size); }
  private:
    Ptr m_begin = nullptr;
    u32_t m_size = 0U;

    inline void assert_size_range(u32_t index) const noexcept {
      J_ASSERT(index <= m_size);
    }
  };

  using memory_region = basic_memory_region<char *>;
  using const_memory_region = basic_memory_region<const char *>;
}
