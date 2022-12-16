#include "rendering/vulkan/rendering/reserved_resource.hpp"

namespace j::rendering::vulkan::rendering {
  namespace detail {
    void reserved_resource_shared_deleter(void * ptr) noexcept {
      J_ASSERT_NOT_NULL(ptr);
      reinterpret_cast<mem::detail::shared_ptr_control_block*>(ptr)->remove_reference();
    }
  }
  reserved_resource & reserved_resource::operator=(reserved_resource && rhs) noexcept {
    if (this != &rhs) {
      if (m_value) {
        m_deleter(m_value);
      }
      m_value = rhs.m_value, m_deleter = rhs.m_deleter;
      rhs.m_value = nullptr, rhs.m_deleter = nullptr;
    }
    return *this;
  }

  reserved_resource::~reserved_resource() {
    if (m_value) {
      m_deleter(m_value);
    }
  }
}
