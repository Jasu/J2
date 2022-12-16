#pragma once

#include "properties/access/typed_access.hpp"
#include "properties/wrappers/wrapper_definition.hpp"
#include "properties/classes/object_property_definition.hpp"
#include "properties/access/list_access.hpp"
#include "properties/access/map_access.hpp"
#include "properties/access/set_access.hpp"
#include "containers/span.hpp"

namespace j::properties::classes {
  class object_property_definition;
}

namespace j::properties::visiting {
  enum class DEFINITION_COMPONENT_TYPE : u8_t {
    /// Root object of the visit.
    ROOT,
    /// Object property definition.
    OBJECT_PROPERTY,
    /// List item definition.
    LIST_ITEM,
    /// Map key definition.
    MAP_KEY,
    /// Map value definition.
    MAP_VALUE,
    /// Set item definition.
    SET_ITEM,
  };


  /// Path of the curretly visited definition item.
  ///
  /// A linked list, entirely stored on stack during the visitation.
  class visit_definition_path {
  public:
    using definitions_t = span<const access::typed_access_definition * const>;
  private:
    const wrappers::wrapper_definition * m_wrapper_definition;
    J_PACKED definitions_t m_typed_access_definitions;
    DEFINITION_COMPONENT_TYPE m_type;
    const visit_definition_path * m_parent;
    const classes::object_property_definition * m_property;

    visit_definition_path(
      DEFINITION_COMPONENT_TYPE type,
      definitions_t access_definitions,
      const wrappers::wrapper_definition * wrapper,
      const visit_definition_path * parent = nullptr,
      const classes::object_property_definition * property = nullptr
    ) : m_wrapper_definition(wrapper),
        m_typed_access_definitions(access_definitions),
        m_type(type),
        m_parent(parent),
        m_property(property)
    {
      J_ASSERT(!m_typed_access_definitions.empty(), "No access definitions to visit.");
      J_ASSERT(m_type == DEFINITION_COMPONENT_TYPE::ROOT || m_parent,
               "Parent is required for non-root components.");
      J_ASSERT(m_type != DEFINITION_COMPONENT_TYPE::OBJECT_PROPERTY || m_property,
               "Property pointer null for an object property.");
    }

    bool path_contains(const wrappers::wrapper_definition * wrapper) const noexcept {
      J_ASSERT_NOT_NULL(wrapper);
      for (auto component = this; component; component = component->m_parent) {
        if (component->m_wrapper_definition == wrapper) {
          return true;
        }
      }
      return false;
    }

  public:
    visit_definition_path(const wrappers::wrapper_definition & wrapper)
      : visit_definition_path(DEFINITION_COMPONENT_TYPE::ROOT,
                              definitions_t(wrapper.possible_contents().begin(),
                                            wrapper.possible_contents().size()),
                              &wrapper)
    { }

    visit_definition_path(const access::typed_access_definition * const & definition)
      : visit_definition_path(DEFINITION_COMPONENT_TYPE::ROOT,
                              definitions_t(&definition, 1U),
                              nullptr)
    { }

    bool path_contains(const classes::object_property_definition & prop) const noexcept {
      return path_contains(prop.definition());
    }

    bool path_contains(const access::list_access_definition & prop) const noexcept {
      return path_contains(prop.item_definition());
    }

    bool path_contains(const access::set_access_definition & prop) const noexcept {
      return path_contains(&prop.item_definition());
    }

    bool path_contains(const access::map_access_definition & prop) const noexcept {
      return path_contains(&prop.value_definition());
    }

    visit_definition_path enter_object_property(
      const classes::object_property_definition & prop
    ) const {
      const auto def = prop.definition();
      return visit_definition_path(
        DEFINITION_COMPONENT_TYPE::OBJECT_PROPERTY,
        definitions_t(def->possible_contents().begin(), def->possible_contents().size()),
        def,
        this,
        &prop);
    }

    visit_definition_path enter_list_item(const access::list_access_definition & list_access) const {
      const auto def = list_access.item_definition();
      return visit_definition_path(
        DEFINITION_COMPONENT_TYPE::LIST_ITEM,
        definitions_t(def->possible_contents().begin(), def->possible_contents().size()),
        def,
        this,
        m_property);
    }

    visit_definition_path enter_set_item(const access::set_access_definition & set_access) const {
      const auto & def = set_access.item_definition();
      return visit_definition_path(
        DEFINITION_COMPONENT_TYPE::SET_ITEM,
        definitions_t(def.possible_contents().begin(), def.possible_contents().size()),
        &def,
        this,
        m_property);
    }

    visit_definition_path enter_map_key(const access::map_access_definition & map_access) const {
      return visit_definition_path(
        DEFINITION_COMPONENT_TYPE::MAP_KEY,
        definitions_t(&map_access.key_definition_ptr(), 1U),
        nullptr,
        this,
        m_property);
    }

    visit_definition_path enter_map_value(const access::map_access_definition & map_access) const {
      const auto & def = map_access.value_definition();
      return visit_definition_path(
        DEFINITION_COMPONENT_TYPE::MAP_VALUE,
        definitions_t(def.possible_contents().begin(), def.possible_contents().size()),
        &def,
        this,
        m_property);
    }

    DEFINITION_COMPONENT_TYPE type() const noexcept {
      return m_type;
    }

    bool is_root() const noexcept {
      return m_type == DEFINITION_COMPONENT_TYPE::ROOT;
    }

    bool is_object_property() const noexcept {
      return m_type == DEFINITION_COMPONENT_TYPE::OBJECT_PROPERTY;
    }

    bool is_list_item() const noexcept {
      return m_type == DEFINITION_COMPONENT_TYPE::LIST_ITEM;
    }

    bool is_map_key() const noexcept {
      return m_type == DEFINITION_COMPONENT_TYPE::MAP_KEY;
    }

    bool is_map_value() const noexcept {
      return m_type == DEFINITION_COMPONENT_TYPE::MAP_VALUE;
    }

    bool is_set_item() const noexcept {
      return m_type == DEFINITION_COMPONENT_TYPE::SET_ITEM;
    }

    const wrappers::wrapper_definition * wrapper_definition() const noexcept {
      return m_wrapper_definition;
    }

    definitions_t access_definitions() const noexcept {
      return m_typed_access_definitions;
    }

    const visit_definition_path * parent() const noexcept {
      return m_parent;
    }

    sz_t size() const noexcept {
      return m_parent ? m_parent->size() + 1 : 1;
    }

    const classes::object_property_definition * property() const noexcept {
      return m_property;
    }
  };
}
