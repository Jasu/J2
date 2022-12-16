#include "properties/conversions/conversion.hpp"

#include "exceptions/assert_lite.hpp"
#include "properties/wrappers/wrapper.hpp"
#include "properties/access/typed_access.hpp"

namespace j::properties::conversions {
  wrappers::wrapper conversion::create_from(
    const wrappers::wrapper_definition & def,
    const void * source
  ) const {
    J_ASSERT_NOT_NULL(source);
    // The reference wrapper contains a pointer, not the real object, thus it cannot be in-place-constructed to.
    return {
      properties::detail::wrapper_take_ownership_tag_t{},
      def,
      m_create_from(source, m_data)
    };
    // }
  }

  void conversion::assign_from(access::typed_access & target, const void * source) const {
    J_ASSERT_NOT_NULL(source);
    m_assign_from(target.as_void_star(), source, m_data);
  }
}
