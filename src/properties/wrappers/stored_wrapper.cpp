#include "properties/wrappers/stored_wrapper.hpp"
#include "properties/access/registry.hpp"

namespace j::properties::wrappers {
  const wrapper_definition & stored_wrapper::from_registry(const std::type_info & type) {
    return *access::registry::get_wrapper_definition(type);
  }

  // stored_wrapper::stored_wrapper(stored_wrapper && rhs) noexcept
  //   : m_data(static_cast<mem::shared_ptr<void> &&>(rhs.m_data)),
  //     m_wrapper(static_cast<class wrapper &&>(rhs.m_wrapper))
  // {
  // }

  // stored_wrapper::stored_wrapper(const stored_wrapper & rhs) noexcept
  //   : m_data(rhs.m_data),
  //     m_wrapper(rhs.m_wrapper)
  // {
  // }

  // stored_wrapper & stored_wrapper::operator=(stored_wrapper && rhs) noexcept {
  //   if (this != &rhs) {
  //     m_data = static_cast<mem::shared_ptr<void> &&>(rhs.m_data);
  //     m_wrapper = static_cast<class wrapper &&>(rhs.m_wrapper);
  //   }
  //   return *this;
  // }

  // stored_wrapper & stored_wrapper::operator=(const stored_wrapper & rhs) noexcept {
  //   if (this != &rhs) {
  //     m_data = rhs.m_data;
  //     m_wrapper = rhs.m_wrapper;
  //   }
  //   return *this;
  // }

  stored_wrapper::~stored_wrapper() {
  }
}
