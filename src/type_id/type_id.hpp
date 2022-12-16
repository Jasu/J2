#pragma once

#include "hzd/utility.hpp"
#include <typeinfo>

#define TYPE_ID_TAG_MASK 0xFFFF000000000000UL
#define TYPE_ID_NAME_MASK 0x0000FFFFFFFFFFFFUL

namespace j::type_id {
  /// Like std::type_info or std::type_index, but sane and less correct.
  ///
  /// GCC does string hashing and string comparison for std::type_info and std::type_index because
  /// loading a dynamic library with RTLD_LOCAL would cause type infos of the same type to be
  /// distinct objects in memory, preventing comparing by pointers. Also, it would break the ABI.
  ///
  /// So type_id is introduced to do the same thing, but by comparing and hashing pointers.
  struct type_id final {
    /// Construct an empty type_id.
    J_A(AI,ND) inline constexpr type_id() noexcept = default;

    /// Construct a type_id from an [std::type_info] object.
    J_A(AI,ND,HIDDEN) inline explicit type_id(const std::type_info & t) noexcept
      : m_data(reinterpret_cast<uptr_t>(t.name()))
    { }

    /// Construct a type_id from an [std::type_info] object and tag bits.
    J_A(AI,ND,HIDDEN) inline type_id(const std::type_info & t, u8_t tag_bits) noexcept
      : m_data(reinterpret_cast<uptr_t>(t.name()) | ((uptr_t)tag_bits << 48))
    { }

    /// Copy-construct while assigning tag bits.
    J_A(AI,ND,HIDDEN) inline constexpr type_id(type_id t, u8_t tag_bits) noexcept
      : m_data((t.m_data & TYPE_ID_NAME_MASK) | ((uptr_t)tag_bits << 48))
    { }

    J_A(AI,ND,HIDDEN) inline type_id & operator=(const std::type_info & rhs) noexcept {
      m_data = reinterpret_cast<uptr_t>(rhs.name()) | (m_data & TYPE_ID_TAG_MASK);
      return *this;
    }

    /// Get the mangled name of the type.
    J_A(AI,NODISC,HIDDEN,ND) inline const char * name() const noexcept {
      return reinterpret_cast<const char *>(m_data & TYPE_ID_NAME_MASK);
    }

    J_A(AI,NODISC,HIDDEN,ND) inline u8_t tag_bits() const noexcept {
      return m_data >> 48;
    }

    J_A(AI,NODISC,HIDDEN,ND) inline bool get_tag_bit(u8_t bit) const noexcept {
      return m_data & (1UL << (48 + bit));
    }

    /// Return true if the type id is empty.
    ///
    /// The type_id is empty if it refers to no type. Set tag bits don't make a
    /// type_id non-empty.
    ///
    /// Unlike [std::type_index] or [std::type_info], type_ids can be empty.
    J_A(AI,NODISC,HIDDEN,ND) constexpr inline bool empty() const noexcept {
      return !(m_data & TYPE_ID_NAME_MASK);
    }

    J_A(AI,NODISC,HIDDEN,ND) constexpr inline bool operator==(type_id rhs) const noexcept {
      return (m_data & TYPE_ID_NAME_MASK) == (rhs.m_data & TYPE_ID_NAME_MASK);
    }

    J_A(AI,NODISC,HIDDEN,ND) inline bool operator<(type_id rhs) const noexcept {
      return (m_data & TYPE_ID_NAME_MASK) < (rhs.m_data & TYPE_ID_NAME_MASK);
    }

    J_A(AI,HIDDEN) inline constexpr void reset() noexcept {
      m_data = 0UL;
    }

    /// Pointer to the name of the type, as returned by std::type_id::name().
    uptr_t m_data = 0;
  };

  J_A(AI,NODISC,HIDDEN,ND) inline bool operator==(type_id lhs, const std::type_info & rhs) noexcept
  { return lhs.name() == rhs.name(); }

  J_A(AI,NODISC,HIDDEN,ND) inline bool operator<(type_id lhs, const std::type_info & rhs) noexcept
  { return lhs.m_data < (uptr_t)rhs.name(); }


  J_A(AI,NODISC,HIDDEN,ND) inline bool operator==(const std::type_info & lhs, type_id rhs) noexcept
  { return lhs.name() == rhs.name(); }

  J_A(AI,NODISC,HIDDEN,ND) inline bool operator<(const std::type_info & lhs, type_id rhs) noexcept
  { return lhs.name() < rhs.name(); }

  using type_id_getter J_NO_DEBUG_TYPE = type_id (*)() noexcept;

  J_A(AI,NODISC) inline constexpr type_id get_empty_type_id() noexcept
  { return type_id(); }

  template<typename T>
  J_A(ND,NODISC) inline type_id get_type_id() noexcept {
    return type_id(typeid(T));
  }
}

#undef TYPE_ID_NAME_MASK
