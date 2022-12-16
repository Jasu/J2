#include "properties/detail/value.hpp"
#include "exceptions/assert_lite.hpp"

namespace j::properties::detail {
  void value_base::copy_from(const value_base & rhs) {
    if (is_in_place_copy()) {
      value_definition().m_copy_construct_in_place(
        reinterpret_cast<u8_t *>(&m_value),
        const_cast<void*>(reinterpret_cast<const void *>(&rhs.m_value)));
    } else if (is_heap_copy()) {
      m_value = value_definition().m_copy_construct(rhs.m_value);
    } else {
      m_value = rhs.m_value;
    }
  }

  void value_base::move_from(value_base && rhs) noexcept {
    if (is_in_place_copy()) {
      value_definition().m_move_construct_in_place(
        reinterpret_cast<u8_t *>(&m_value),
        reinterpret_cast<void*>(&rhs.m_value));
      rhs.clear();
    } else if (is_heap_copy()) {
      m_value = value_definition().m_move_construct(rhs.m_value);
      rhs.clear();
    } else {
      m_value = rhs.m_value;
    }
  }

  value_base & value_base::operator=(const value_base  & rhs) {
    if (this != &rhs) {
      if ((m_definition & ~0x07UL) == (rhs.m_definition & ~0x07UL)) {
        value_definition().m_copy_assign(as_void_star(), const_cast<void*>(rhs.as_void_star()));
      } else {
        clear();
        m_definition = rhs.m_definition;
        copy_from(rhs);
      }
    }
    return *this;
  }

  value_base & value_base::operator=(value_base && rhs) noexcept {
    if (this != &rhs) {
      if ((m_definition & ~0x07UL) == (rhs.m_definition & ~0x07UL)) {
        value_definition().m_move_assign(as_void_star(), rhs.as_void_star());
        rhs.clear();
      } else {
        release();
        m_definition = rhs.m_definition;
        move_from(static_cast<value_base &&>(rhs));
      }
    }
    return *this;
  }

  void value_definition::initialize(const value_definition & rhs) noexcept {
    m_delete = rhs.m_delete;
    m_destruct = rhs.m_destruct;
    m_copy_construct = rhs.m_copy_construct;
    m_move_construct = rhs.m_move_construct;
    m_copy_construct_in_place = rhs.m_copy_construct_in_place;
    m_move_construct_in_place = rhs.m_move_construct_in_place;
    m_copy_assign = rhs.m_copy_assign;
    m_move_assign = rhs.m_move_assign;
    m_in_place_size = rhs.m_in_place_size;
  }

  value_base::value_base(const wrapper_copy_tag_t &, const class value_definition & def, const void * pointer)
    : m_definition(reinterpret_cast<uptr_t>(&def) | 0x1UL | (def.in_place_size() > sizeof(void*) ? 0x2UL : 0x4UL))
  {
    J_ASSERT_NOT_NULL(def.m_delete);
    if (def.in_place_size() <= sizeof(void*)) {
      def.m_copy_construct_in_place(&m_value, const_cast<void*>(pointer));
    } else {
      m_value = def.m_copy_construct(const_cast<void*>(pointer));
    }
  }

  value_base::value_base(const access_copy_tag_t &, const class value_definition & def, const void * pointer)
    : m_definition(reinterpret_cast<uptr_t>(&def) | (def.in_place_size() > sizeof(void*) ? 0x2UL : 0x4UL))
  {
    J_ASSERT_NOT_NULL(def.m_delete);
    if (def.in_place_size() <= sizeof(void*)) {
      def.m_copy_construct_in_place(&m_value, const_cast<void*>(pointer));
    } else {
      m_value = def.m_copy_construct(const_cast<void*>(pointer));
    }
  }
  void value_base::do_release() noexcept {
    auto def = reinterpret_cast<const class value_definition *>(m_definition & ~0x7UL);
    if (m_definition & 0x02) {
      def->m_delete(m_value);
    } else if (m_definition & 0x04 && def->m_destruct) {
      def->m_destruct(m_value);
    }
  }

  namespace detail {
    void default_ptr_destructor(void *) noexcept { }
    void default_ptr_deleter(void * ptr) noexcept {
      delete reinterpret_cast<void**>(ptr);
    }
    void * default_ptr_copy_construct(void * ptr) {
      return new void*(*reinterpret_cast<void**>(ptr));
    }
    void default_ptr_copy_construct_in_place(void * to, void * ptr) noexcept {
      *reinterpret_cast<void**>(to) = *reinterpret_cast<void**>(ptr);
    }
    void default_ptr_copy_assign(void * to, void * ptr) {
      *reinterpret_cast<void**>(to) = *reinterpret_cast<void**>(ptr);
    }
  }
}
