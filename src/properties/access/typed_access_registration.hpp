#pragma once

#include "type_id/type_id.hpp"
#include "containers/trivial_array_fwd.hpp"

J_DECLARE_EXTERN_TRIVIAL_ARRAY(j::type_id::type_id);

namespace j::properties::conversions {
  class conversion_definition;
}
namespace j::properties::wrappers {
  class wrapper_definition;
}
namespace j::properties::access {
  class typed_access;
  class typed_access_definition;

  class typed_access_registration final {
  public:
    typed_access_registration() noexcept;

    void add_to_registry(const std::type_info & type, typed_access_definition * def, const wrappers::wrapper_definition * wrapper_def);

    ~typed_access_registration();
    typed_access_registration(const typed_access_registration &) = delete;
    typed_access_registration & operator=(const typed_access_registration &) = delete;

    void set_num_conversions(sz_t num);
    void add_conversion_to(const conversions::conversion_definition & conversion);

    template<typename... Conversions>
    J_HIDDEN J_ALWAYS_INLINE void operator()(const Conversions & ... convs);

  private:
    type_id::type_id m_type;
    trivial_array<type_id::type_id> m_conversions_to;
    bool m_is_registered = false;
  };

  template<typename... Conversions>
  J_ALWAYS_INLINE void typed_access_registration::operator()(const Conversions & ... convs) {
    set_num_conversions(sizeof...(convs));
    (add_conversion_to(convs), ...);
  }
}
