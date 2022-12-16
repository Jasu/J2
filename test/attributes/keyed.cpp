#include <detail/preamble.hpp>

#include "attributes/attributes.hpp"
#include "attributes/trivial_array.hpp"
#include "strings/string.hpp"
#include "containers/trivial_array.hpp"
#include "containers/pair.hpp"

TEST_SUITE_BEGIN("Attributes - Keyed");

using namespace j::attributes;
using namespace j;

namespace {
  const attribute_definition attr_keyed(
    value_type = type<pair<strings::string, int>>,
    key_type = type<strings::string>,
    tag = type<struct keyed_tag>,
    is_multiple
  );

  template<typename... Attributes>
  void require_keyed(const strings::string & key, int value, Attributes && ... attrs) {
    auto a = as_trivial_array_move<pair<strings::string, int>>(
      attr_keyed, static_cast<Attributes &&>(attrs)...);
    REQUIRE(a.size() == 1);
    REQUIRE(a[0].first == key);
    REQUIRE(a[0].second == value);
  }
}

TEST_CASE("attribute - keyed") {
  require_keyed("hello", 123, attr_keyed["hello"] = 123);
}

TEST_SUITE_END();
