#pragma once

#include "hzd/utility.hpp"

namespace j::bits {
  /// Iterator for iterating a bitmask as a set.
  ///
  /// The iterator has a start offset for the first bit in the pointer, i.e. the bit field does
  /// not have to start at bit 0.
  template<typename FieldType, typename EnumType>
  class bitmask_iterator_with_offset {
    J_NO_DEBUG static constexpr const u8_t offset_mask = sizeof(FieldType) * 8U - 1U;

    const FieldType * m_ptr;
    u8_t m_offset;
    u8_t m_end_offset;
    u8_t m_start_offset;

    void increment() noexcept;

    J_ALWAYS_INLINE J_HIDDEN void advance() noexcept {
      while (m_offset != m_end_offset && !(*m_ptr & (1ULL << (m_offset & offset_mask)))) {
        increment();
      }
    }
  public:
    bitmask_iterator_with_offset() noexcept;

    /// Construct a bitmask_iterator usable only as an end iterator.
    ///
    /// The iterator can be compared against other iterators, but any other operations will result
    /// in undefined behavior.
    J_ALWAYS_INLINE J_HIDDEN explicit bitmask_iterator_with_offset(u8_t offset) noexcept
      : m_offset(offset)
    {
    }

    J_ALWAYS_INLINE J_HIDDEN bitmask_iterator_with_offset(
      const FieldType * ptr,
      u8_t offset,
      u8_t start_offset,
      u8_t end_offset
    ) noexcept
    : m_ptr(ptr),
      m_offset(offset),
      m_end_offset(end_offset),
      m_start_offset(start_offset)
    {
      advance();
    }

    J_ALWAYS_INLINE J_HIDDEN EnumType operator*() const noexcept {
      return static_cast<EnumType>(m_offset - m_start_offset);
    }

    bitmask_iterator_with_offset & operator++() noexcept;

    bitmask_iterator_with_offset & operator++(int) noexcept;

    J_ALWAYS_INLINE J_HIDDEN bool operator==(const bitmask_iterator_with_offset & rhs) const noexcept {
      return m_offset == rhs.m_offset;
    }

    J_ALWAYS_INLINE J_HIDDEN bool operator!=(const bitmask_iterator_with_offset & rhs) const noexcept {
      return m_offset != rhs.m_offset;
    }


    J_ALWAYS_INLINE J_HIDDEN const FieldType * ptr() const noexcept {
      return m_ptr;
    }

    J_ALWAYS_INLINE J_HIDDEN u8_t offset() const noexcept {
      return m_offset;
    }
  };

  template<typename FieldType, typename EnumType>
  bitmask_iterator_with_offset<FieldType, EnumType>::bitmask_iterator_with_offset() noexcept {
  }


  template<typename FieldType, typename EnumType>
  bitmask_iterator_with_offset<FieldType, EnumType> & bitmask_iterator_with_offset<FieldType, EnumType>::operator++() noexcept {
    increment();
    advance();
    return *this;
  }

  template<typename FieldType, typename EnumType>
  J_ALWAYS_INLINE J_HIDDEN bitmask_iterator_with_offset<FieldType, EnumType> & bitmask_iterator_with_offset<FieldType, EnumType>::operator++(int) noexcept {
    bitmask_iterator_with_offset result = *this;
    ++*this;
    return result;
  }

  template<typename FieldType, typename EnumType>
  void bitmask_iterator_with_offset<FieldType, EnumType>::increment() noexcept {
    ++m_offset;
    if (!(m_offset & offset_mask)) {
      ++m_ptr;
    }
  }
}
