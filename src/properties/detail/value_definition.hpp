#pragma once

#include "hzd/types.hpp"

namespace j::properties::detail {
  class value_definition {
    friend class value_base;
    friend class value;
  public:
    using delete_t = void (*)(void *) noexcept;
    using copy_construct_t = void * (*)(void *);
    using copy_construct_in_place_t = void (*)(void *, void *);
    using copy_assign_t = void (*)(void *, void*);
  private:
    delete_t m_delete = nullptr;
    delete_t m_destruct = nullptr;
    copy_construct_t m_copy_construct = nullptr;
    copy_construct_t m_move_construct = nullptr;
    copy_construct_in_place_t m_copy_construct_in_place = nullptr;
    copy_construct_in_place_t m_move_construct_in_place = nullptr;
    copy_assign_t m_copy_assign = nullptr;
    copy_assign_t m_move_assign = nullptr;
    sz_t m_in_place_size = SZ_MAX;
  public:
    constexpr value_definition() noexcept = default;

    void initialize(const value_definition & rhs) noexcept;

    constexpr value_definition(
      delete_t deleter,
      delete_t destruct,
      copy_construct_t copy_construct,
      copy_construct_t move_construct,
      copy_construct_in_place_t copy_construct_in_place,
      copy_construct_in_place_t move_construct_in_place,
      copy_assign_t copy_assign,
      copy_assign_t move_assign,
      sz_t in_place_size) noexcept;

    constexpr sz_t in_place_size() const noexcept {
      return m_in_place_size;
    }
  };
}
