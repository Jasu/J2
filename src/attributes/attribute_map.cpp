#include "attributes/attribute_map.hpp"
#include "containers/hash_map.hpp"
#include "exceptions/assert.hpp"

J_DEFINE_EXTERN_HASH_MAP(j::type_id::type_id, j::noncopyable_vector<j::mem::any>, HASH(j::type_id::hash));

namespace j::attributes {
  attribute_map::attribute_map() noexcept { }

  attribute_map::attribute_map(attribute_map && rhs) noexcept
    : m_map(static_cast<hash_map<type_id::type_id, noncopyable_vector<mem::any>, type_id::hash> &&>(rhs.m_map)) { }

  attribute_map & attribute_map::operator=(attribute_map && rhs) noexcept {
    m_map = static_cast<hash_map<type_id::type_id, noncopyable_vector<mem::any>, type_id::hash> &&>(rhs.m_map);
    return *this;
  }

  attribute_map::~attribute_map() {
  }

  bool attribute_map::empty() const noexcept {
    return m_map.empty();
  }

  u32_t attribute_map::size() const noexcept {
    return m_map.size();
  }

  auto attribute_map::begin() noexcept {
    return m_map.begin();
  }

  auto attribute_map::end() noexcept {
    return m_map.end();
  }

  auto attribute_map::begin() const noexcept {
    return m_map.begin();
  }

  auto attribute_map::end() const noexcept {
    return m_map.end();
  }

  bool attribute_map::contains_type(const std::type_info & type) const noexcept {
    return m_map.contains(type_id::type_id(type));
  }

  u32_t attribute_map::count_type(const std::type_info & type) const noexcept {
    if (auto p = m_map.maybe_at(type_id::type_id(type))) {
      return p->size();
    }
    return 0U;
  }

  [[nodiscard]] mem::any & attribute_map::first_type(const std::type_info & type) noexcept {
    return m_map.at(type_id::type_id(type)).front();
  }
  [[nodiscard]] mem::any const & attribute_map::first_type(const std::type_info & type) const noexcept {
    return m_map.at(type_id::type_id(type)).front();
  }

  [[nodiscard]] mem::any * attribute_map::maybe_first_type(const std::type_info & type) noexcept {
    if (auto p = m_map.maybe_at(type_id::type_id(type))) {
      return p->begin();
    }
    return nullptr;
  }

  [[nodiscard]] mem::any const * attribute_map::maybe_first_type(const std::type_info & type) const noexcept {
    if (auto p = m_map.maybe_at(type_id::type_id(type))) {
      return p->begin();
    }
    return nullptr;
  }

  span<mem::any> attribute_map::maybe_at_type(const std::type_info & type) noexcept {
    if (auto p = m_map.maybe_at(type_id::type_id(type))) {
      return *p;
    }
    return {};
  }

  span<const mem::any> attribute_map::maybe_at_type(const std::type_info & type) const noexcept {
    if (auto p = m_map.maybe_at(type_id::type_id(type))) {
      return *p;
    }
    return {};
  }

  void attribute_map::emplace(const std::type_info & type, mem::any && value) {
    m_map[type_id::type_id(type)].emplace_back(static_cast<mem::any &&>(value));
  }
}
