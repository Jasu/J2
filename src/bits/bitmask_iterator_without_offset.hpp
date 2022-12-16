#pragma once

#include "hzd/types.hpp"

namespace j::bits {
  /// Iterator for iterating a bitmask as a set.
  ///
  /// The bit field has to start at bit 0.
  template<typename FieldType, typename EnumType>
  class bitmask_iterator_without_offset {
    static constexpr const u8_t offset_mask = sizeof(FieldType) * 8 - 1;

    const FieldType * m_ptr;
    u8_t m_offset;
    u8_t m_end_offset;

    void increment() noexcept;
    void advance() noexcept;
  public:
    const FieldType * ptr() const noexcept;

    u8_t offset() const noexcept;

    bitmask_iterator_without_offset() noexcept = default;

    /// Construct a bitmask_iterator usable only as an end iterator.
    ///
    /// The iterator can be compared against other iterators, but any other operations will result
    /// in undefined behavior.
    explicit bitmask_iterator_without_offset(u8_t offset) noexcept;

    explicit bitmask_iterator_without_offset(
      const FieldType * ptr,
      u8_t offset,
      u8_t end_offset
    ) noexcept;

    EnumType operator*() const noexcept;

    bitmask_iterator_without_offset & operator++() noexcept;

    bitmask_iterator_without_offset & operator++(int) noexcept;

    bool operator==(const bitmask_iterator_without_offset & rhs) const noexcept;

    bool operator!=(const bitmask_iterator_without_offset & rhs) const noexcept;
  };

  template<typename FieldType, typename EnumType>
  const FieldType * bitmask_iterator_without_offset<FieldType, EnumType>::ptr() const noexcept {
    return m_ptr;
  }

  template<typename FieldType, typename EnumType>
  u8_t bitmask_iterator_without_offset<FieldType, EnumType>::offset() const noexcept {
    return m_offset;
  }

  template<typename FieldType, typename EnumType>
  bitmask_iterator_without_offset<FieldType, EnumType>::bitmask_iterator_without_offset(u8_t offset) noexcept : m_offset(offset) { }

  template<typename FieldType, typename EnumType>
  bitmask_iterator_without_offset<FieldType, EnumType>::bitmask_iterator_without_offset(
    const FieldType * ptr,
    u8_t offset,
    u8_t end_offset
    ) noexcept
    : m_ptr(ptr),
      m_offset(offset),
      m_end_offset(end_offset)
  {
    advance();
  }

  template<typename FieldType, typename EnumType>
  EnumType bitmask_iterator_without_offset<FieldType, EnumType>::operator*() const noexcept {
    return static_cast<EnumType>(m_offset);
  }

  template<typename FieldType, typename EnumType>
  bitmask_iterator_without_offset<FieldType, EnumType> & bitmask_iterator_without_offset<FieldType, EnumType>::operator++() noexcept {
    increment();
    advance();
    return *this;
  }

  template<typename FieldType, typename EnumType>
  bitmask_iterator_without_offset<FieldType, EnumType> & bitmask_iterator_without_offset<FieldType, EnumType>::operator++(int) noexcept {
    bitmask_iterator_without_offset result = *this;
    ++*this;
    return result;
  }

  template<typename FieldType, typename EnumType>
  bool bitmask_iterator_without_offset<FieldType, EnumType>::operator==(const bitmask_iterator_without_offset & rhs) const noexcept {
    return m_offset == rhs.m_offset;
  }

  template<typename FieldType, typename EnumType>
  bool bitmask_iterator_without_offset<FieldType, EnumType>::operator!=(const bitmask_iterator_without_offset & rhs) const noexcept {
    return m_offset != rhs.m_offset;
  }

  template<typename FieldType, typename EnumType>
  void bitmask_iterator_without_offset<FieldType, EnumType>::increment() noexcept {
    ++m_offset;
    if (!(m_offset & offset_mask)) {
      ++m_ptr;
    }
  }

  template<typename FieldType, typename EnumType>
  void bitmask_iterator_without_offset<FieldType, EnumType>::advance() noexcept {
    while (m_offset != m_end_offset && !(*m_ptr & (1ULL << (m_offset & offset_mask)))) {
      increment();
    }
  }
}
