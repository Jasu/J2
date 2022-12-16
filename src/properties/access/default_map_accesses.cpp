#include "properties/access/default_accesses.hpp"
#include "properties/property_variant.hpp"
#include "properties/access/string_access.hpp"
#include "properties/access/map_access_registration.hpp"
#include "properties/wrappers/variant_wrapper.hpp"
#include "properties/access/typed_access_registration.hpp"
#include "containers/unsorted_string_map.hpp"

namespace j::properties::access {
  namespace {
    struct variant_map_iterator_functions : default_map_iterator_functions<map_t> {
      J_ALWAYS_INLINE J_HIDDEN static const strings::string & get_iterator_key(iterator_data & it) {
        return *iter(it)->first;
      }

      J_ALWAYS_INLINE J_HIDDEN static property_variant & get_iterator_value(iterator_data & it) {
        return *iter(it)->second;
      }
    };

    struct variant_map_access : associative_container_access_wrapper<map_t, variant_map_iterator_functions> {
      J_ALWAYS_INLINE J_HIDDEN static iterator_data replace(map_t & c, const strings::string & key, const property_variant & value) {
        auto result = c.emplace(key, value);
        if (!result.second) {
          *result.first->second = value;
        }
        return variant_map_iterator_functions::wrap_iterator(static_cast<map_t::iterator &&>(result.first));
      }

    };

    using registration_t = map_access_registration<map_t, variant_map_access>;

    registration_t variant_map_registration("map_t");
  }

  const typed_access_definition * const variant_map_access_definition = &registration_t::definition;
  const wrappers::wrapper_definition * const variant_map_wrapper_definition = &registration_t::wrapper;
}
