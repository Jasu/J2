#include "properties/access/default_accesses.hpp"
#include "properties/access/list_access_registration.hpp"
#include "properties/wrappers/variant_wrapper.hpp"
#include "properties/property_variant.hpp"
#include "containers/vector.hpp"

namespace j::properties::access {
  using list_t_registration_t = list_access_registration<list_t>;
  static list_t_registration_t variant_list_registration("list_t");
  const typed_access_definition * const variant_list_access_definition = &list_t_registration_t::definition;
  const wrappers::wrapper_definition * const variant_list_wrapper_definition = &list_t_registration_t::wrapper;

  using j_strvec_registration_t = list_access_registration<vector<strings::string>>;
  static j_strvec_registration_t j_strvec_list_registration("vector<strings::string>");
  const typed_access_definition * const j_string_vector_access_definition = &j_strvec_registration_t::definition;
  const wrappers::wrapper_definition * const j_string_vector_wrapper_definition = &j_strvec_registration_t::wrapper;
}
