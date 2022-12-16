#pragma once

#include "properties/property_type.hpp"
#include "properties/detail/value_definition.hpp"
#include "containers/trivial_array_fwd.hpp"

namespace j::properties::access {
  class typed_access;
  class typed_access_definition;
}
J_DECLARE_EXTERN_TRIVIAL_ARRAY(const j::properties::access::typed_access_definition *);

namespace j::properties::wrappers {
  class wrapper;

  class wrapper_definition : public properties::detail::value_definition {
    friend class wrapper;
  public:
    using get_current_type_t = PROPERTY_TYPE (*)(const void *);
    using set_value_t = void (*)(void *, const access::typed_access &);
    using get_value_t = access::typed_access (*)(void *);
    using create_t = wrapper (*)(const wrappers::wrapper_definition &, access::typed_access &);

  protected:
    get_current_type_t m_get_current_type = nullptr;
    set_value_t m_set_value = nullptr;
    get_value_t m_get_value = nullptr;
    create_t m_create = nullptr;
    trivial_array<const access::typed_access_definition *> m_possible_contents;

  public:
    wrapper_definition() noexcept;

    wrapper_definition(properties::detail::value_definition && value_definition,
                       get_current_type_t get_current_type,
                       set_value_t set_value,
                       get_value_t get_value,
                       create_t create,
                       trivial_array<const access::typed_access_definition *> && possible_contents);

    wrapper_definition(properties::detail::value_definition && value_definition,
                       PROPERTY_TYPE property_type,
                       set_value_t set_value,
                       get_value_t get_value,
                       create_t create,
                       const access::typed_access_definition * type);

    ~wrapper_definition();

    wrapper create(const access::typed_access & access) const;

    J_INLINE_GETTER const trivial_array<const access::typed_access_definition *> & possible_contents() const noexcept
    { return m_possible_contents; }

    wrapper convert_from(const access::typed_access & item) const;

    bool can_contain(const access::typed_access_definition * def) const noexcept;
  };
}
