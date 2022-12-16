#pragma once

#include "properties/detail/value_definition.hpp"
#include "hzd/utility.hpp"

namespace j::properties::detail {
  enum class wrapper_tag_t { v };
  enum class access_tag_t { v };
  enum class access_in_place_tag_t { v };
  enum class wrapper_copy_tag_t { v };
  enum class access_copy_tag_t { v };
  enum class wrapper_in_place_tag_t { v };
  enum class wrapper_take_ownership_tag_t { v };
  enum class access_take_ownership_tag_t { v };

  J_NO_DEBUG inline constexpr wrapper_tag_t wrapper_tag = wrapper_tag_t::v;
  J_NO_DEBUG inline constexpr access_tag_t access_tag = access_tag_t::v;
  J_NO_DEBUG inline constexpr access_in_place_tag_t access_in_place_tag = access_in_place_tag_t::v;
  J_NO_DEBUG inline constexpr wrapper_copy_tag_t wrapper_copy_tag = wrapper_copy_tag_t::v;
  J_NO_DEBUG inline constexpr access_copy_tag_t access_copy_tag = access_copy_tag_t::v;
  J_NO_DEBUG inline constexpr wrapper_in_place_tag_t wrapper_in_place_tag = wrapper_in_place_tag_t::v;
  J_NO_DEBUG inline constexpr wrapper_take_ownership_tag_t wrapper_take_ownership_tag = wrapper_take_ownership_tag_t::v;
  J_NO_DEBUG inline constexpr access_take_ownership_tag_t access_take_ownership_tag = access_take_ownership_tag_t::v;

  class value_base {
  private:
    uptr_t m_definition = 0;
    void * m_value = nullptr;

    void copy_from(const value_base & rhs);

    void move_from(value_base && rhs) noexcept;
  protected:

    explicit value_base(const value_definition * definition, void * value = nullptr) noexcept
      : m_definition(reinterpret_cast<uptr_t>(definition)),
        m_value(value)
    {
    }

    explicit value_base(const wrapper_tag_t &, const class value_definition & def, void * pointer = nullptr) noexcept
      : m_definition(reinterpret_cast<uptr_t>(&def) | 0x01UL),
        m_value(pointer)
    { }

    explicit value_base(const access_tag_t &, const class value_definition & def, void * pointer = nullptr) noexcept
      : m_definition(reinterpret_cast<uptr_t>(&def)),
        m_value(pointer)
    { }

    explicit value_base(const wrapper_copy_tag_t &, const class value_definition & def, const void * pointer);

    explicit value_base(const wrapper_in_place_tag_t &, const class value_definition & def, void * value = nullptr) noexcept
      : m_definition(reinterpret_cast<uptr_t>(&def) | 0x5UL),
        m_value(value)
    {
    }

    explicit value_base(const access_in_place_tag_t &, const class value_definition & def, void * value = nullptr) noexcept
      : m_definition(reinterpret_cast<uptr_t>(&def) | 0x4UL),
        m_value(value)
    {
    }

    explicit value_base(const access_copy_tag_t &, const class value_definition & def, const void * pointer);

    explicit value_base(const wrapper_take_ownership_tag_t &, const class value_definition & def, void * pointer) noexcept
      : m_definition(reinterpret_cast<uptr_t>(&def) | 0x3UL),
        m_value(pointer)
    {
    }

    explicit value_base(const access_take_ownership_tag_t &, const class value_definition & def, void * pointer) noexcept
      : m_definition(reinterpret_cast<uptr_t>(&def) | 0x2UL),
        m_value(pointer)
    {
    }

  public:
    constexpr value_base() noexcept = default;

    value_base(const value_base & rhs)
      : m_definition(rhs.m_definition)
    {
      copy_from(rhs);
    }

    value_base(value_base && rhs) noexcept
      : m_definition(rhs.m_definition)
    {
      move_from(static_cast<value_base &&>(rhs));
    }

    value_base & operator=(const value_base & rhs);

    value_base & operator=(value_base && rhs) noexcept;

    const void * as_void_star() const noexcept {
      return (m_definition & 0x04UL) ? &m_value : m_value;
    }

    void * as_void_star() noexcept {
      return (m_definition & 0x04UL) ? &m_value : m_value;
    }

    template<typename T>
    J_HIDDEN J_ALWAYS_INLINE const T & as_raw_ref() const noexcept {
      return *reinterpret_cast<const T*>(as_void_star());
    }

    template<typename T>
    J_HIDDEN J_ALWAYS_INLINE T & as_raw_ref() noexcept {
      return *reinterpret_cast<T*>(as_void_star());
    }

    bool is_heap_copy() const noexcept {
      return m_definition & 0x02U;
    }

    bool is_in_place_copy() const noexcept {
      return m_definition & 0x04U;
    }

    void do_release() noexcept;

    void release() noexcept {
      if (m_definition & 0x06U) {
        do_release();
      }
    }

    void clear() noexcept {
      release();
      m_definition = 0U;
    }

    bool empty() const noexcept {
      return !m_definition;
    }

    ~value_base() {
      if (m_definition & 0x06U) {
        do_release();
      }
    }

    const class value_definition & value_definition() const noexcept {
      return *reinterpret_cast<const class value_definition *>(m_definition & ~0x7UL);
    }
  };
}
