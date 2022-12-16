#pragma once

#include "properties/access/typed_access.hpp"
#include "properties/wrappers/wrapper.hpp"
#include "properties/classes/object_property_definition.hpp"
#include "containers/unsorted_string_map_fwd.hpp"

J_DECLARE_EXTERN_UNSORTED_STRING_MAP(j::properties::classes::object_property_getter);
J_DECLARE_EXTERN_UNSORTED_STRING_MAP(j::properties::classes::object_property_definition);

#include "object_iterator.hpp"

namespace j::properties::wrappers {
  class wrapper;
}

namespace j::properties::classes {
  class object_access_definition final : public access::typed_access_definition {
    friend class object_access;
    template<typename, typename>
    friend class object_access_registration;
  public:
    using property_accessors_t J_NO_DEBUG_TYPE = uncopyable_unsorted_string_map<object_property_getter>;
    using properties_t J_NO_DEBUG_TYPE = uncopyable_unsorted_string_map<object_property_definition>;
  private:
    /// Map of property accessors, keyed by their names.
    property_accessors_t m_property_accessors;
    /// Map of property definitions, keyed by their names.
    properties_t m_properties;
  public:
    object_access_definition() noexcept;
    void on_property_wrapper_available(object_property_definition* prop, const wrappers::wrapper_definition * def);

    ~object_access_definition();

    void initialize_definition(object_property_definition && def);
    void initialize_object_access_done();

    properties_t & get_properties() noexcept {
      return m_properties;
    }

    const properties_t & get_properties() const noexcept {
      return m_properties;
    }

    const object_property_definition & get_property(const strings::string & str) const;

    span<const strings::string> property_names() const noexcept {
      return m_properties.keys();
    }
  };

  class object_access final : public access::typed_access {
  public:
    using access::typed_access::typed_access;

    template<typename String>
    J_HIDDEN wrappers::wrapper get_property(const String & name) {
      return static_cast<const object_access_definition &>(value_definition()).m_property_accessors.at(name).get(as_void_star());
    }

    const object_access_definition::properties_t & get_properties() const noexcept {
      return static_cast<const object_access_definition &>(value_definition()).m_properties;
    }

    span<const strings::string> property_names() const noexcept {
      return static_cast<const object_access_definition &>(value_definition()).property_names();
    }

    object_iterator begin() noexcept;

    object_iterator end() noexcept;
  };
}
