#include <detail/preamble.hpp>
#include <typeinfo>

#include "properties/wrappers/variant_wrapper.hpp"
#include "properties/wrappers/reference_wrapper.hpp"
#include "properties/access/string_access.hpp"
#include "properties/access/int_access.hpp"
#include "properties/access/float_access.hpp"
#include "properties/access/bool_access.hpp"
#include "properties/access/list_access.hpp"
#include "properties/access/map_access.hpp"
#include "properties/property_variant.hpp"

template<typename T>
inline void test_typed(j::properties::wrappers::wrapper & w, j::properties::PROPERTY_TYPE t, T) {
  REQUIRE(w.type() == t);
  REQUIRE(w.get_value().type_id() == typeid(T));
}

// inline void test_assign_from_same_type(j::properties::property_variant lhs, j::properties::property_variant rhs) {
//   j::properties::access::property_access lhs_a(lhs);
//   j::properties::access::property_access rhs_a(rhs);
//   lhs_a.assign_from_same_type(rhs_a);
//   REQUIRE(lhs_a.type() == rhs_a.type());
//   REQUIRE(lhs_a.type() == lhs.type());
//   REQUIRE(lhs == rhs);
// }

TEST_CASE("Empty property variant access") {
  j::properties::property_variant v;
  auto w = j::properties::wrappers::access(v);
  REQUIRE(w.type() == j::properties::PROPERTY_TYPE::EMPTY);
}

TEST_CASE("Int property variant access") {
  j::properties::property_variant v = 2;
  auto w = j::properties::wrappers::access(v);
  test_typed<j::properties::int_t>(w, j::properties::PROPERTY_TYPE::INT, 2);
}

TEST_CASE("Float property variant access") {
  j::properties::property_variant v = 2.0;
  auto w = j::properties::wrappers::access(v);
  test_typed<j::properties::float_t>(w, j::properties::PROPERTY_TYPE::FLOAT, 2.0);
}

TEST_CASE("Bool property variant access") {
  j::properties::property_variant v = false;
  auto w = j::properties::wrappers::access(v);
  test_typed<j::properties::bool_t>(w, j::properties::PROPERTY_TYPE::BOOL, false);
}

TEST_CASE("Nil property variant access") {
  j::properties::property_variant v = nullptr;
  auto w = j::properties::wrappers::access(v);
  test_typed<j::properties::nil_t>(w, j::properties::PROPERTY_TYPE::NIL, nullptr);
}

TEST_CASE("List property variant access") {
  j::properties::property_variant v = j::properties::list_t{};
  auto w = j::properties::wrappers::access(v);
  test_typed<j::properties::list_t>(w, j::properties::PROPERTY_TYPE::LIST, j::properties::list_t{});
}


TEST_CASE("Map property variant access") {
  j::properties::property_variant v = j::properties::map_t{};
  auto w = j::properties::wrappers::access(v);
  test_typed<j::properties::map_t>(w, j::properties::PROPERTY_TYPE::MAP, j::properties::map_t{});
}

// TEST_CASE("Assign variant access") {
//   j::properties::property_variant variants[] = { 2, 2.0, false, nullptr, j::properties::map_t{}, j::properties::list_t{} };
//   for (sz_t i = 0; i < 6; ++i) {
//     for (sz_t j = 0; j < 6; ++j) {
//       test_assign_from_same_type(variants[i], variants[j]);
//     }
//   }
// }

// TEST_CASE("Create from variant access") {
//   {
//     auto def = j::properties::access::variant_access_definition.create_default();
//     REQUIRE(def->type() == j::properties::PROPERTY_TYPE::EMPTY);
//   }

//   {
//     j::strings::string s{"hello"};
//     j::properties::access::property_access a{s};
//     auto def = j::properties::access::variant_access_definition.create_from(a);
//     REQUIRE(def->type() == j::properties::PROPERTY_TYPE::STRING);
//     REQUIRE(def->as_string().as_j_string() == "hello");
//   }

//   {
//     int i = 4;
//     j::properties::access::property_access a{i};
//     auto def = j::properties::access::variant_access_definition.create_from(a);
//     REQUIRE(def->type() == j::properties::PROPERTY_TYPE::INT);
//     REQUIRE(def->as_int().get_value() == 4);
//     def->as_int().set_value(0);
//     REQUIRE(def->as_int().get_value() == 0);
//     REQUIRE(i == 4);
//   }

//   {
//     float f = 4.0;
//     j::properties::access::property_access a{f};
//     auto def = j::properties::access::variant_access_definition.create_from(a);
//     REQUIRE(def->type() == j::properties::PROPERTY_TYPE::FLOAT);
//     REQUIRE(def->as_float().get_value() == 4.0);
//     def->as_float().set_value(7.5);
//     REQUIRE(def->as_float().get_value() == 7.5);
//     REQUIRE(f == 4.0);
//   }

//   {
//     bool b = false;
//     j::properties::access::property_access a{b};
//     auto def = j::properties::access::variant_access_definition.create_from(a);
//     REQUIRE(def->type() == j::properties::PROPERTY_TYPE::BOOL);
//     REQUIRE(def->as_bool().get_value() == false);
//     def->as_bool().set_value(true);
//     REQUIRE(def->as_bool().get_value() == true);
//     REQUIRE(b == false);
//   }

//   {
//     j::properties::map_t m{};
//     m.emplace("k1", 1);
//     j::properties::access::property_access a{m};
//     auto def = j::properties::access::variant_access_definition.create_from(a);
//     REQUIRE(def->type() == j::properties::PROPERTY_TYPE::MAP);
//     REQUIRE(def->as_map().size() == 1);
//     def->as_map().clear();
//     REQUIRE(def->as_map().size() == 0);
//     REQUIRE(m.size() == 1);
//   }

//   {
//     j::properties::list_t l{};
//     l.push_back(1);
//     j::properties::access::property_access a{l};
//     auto def = j::properties::access::variant_access_definition.create_from(a);
//     REQUIRE(def->type() == j::properties::PROPERTY_TYPE::LIST);
//     REQUIRE(def->as_list().size() == 1);
//     def->as_list().clear();
//     REQUIRE(def->as_list().size() == 0);
//     REQUIRE(l.size() == 1);
//   }
// }

// TEST_CASE("Assign from variant access") {
//   j::properties::property_variant v;
//   j::properties::access::property_access a{v};
//   {
//     j::properties::list_t l{};
//     j::properties::access::property_access la{l};
//     a.assign_from(la);
//     REQUIRE(v.type() == j::properties::PROPERTY_TYPE::LIST);
//   }

//   {
//     j::properties::map_t m{};
//     j::properties::access::property_access ma{m};
//     a.assign_from(ma);
//     REQUIRE(v.type() == j::properties::PROPERTY_TYPE::MAP);
//   }

//   {
//     bool b = false;
//     j::properties::access::property_access ba{b};
//     a.assign_from(ba);
//     REQUIRE(v.type() == j::properties::PROPERTY_TYPE::BOOL);
//   }

//   {
//     float f = 1.0f;
//     j::properties::access::property_access fa{f};
//     a.assign_from(fa);
//     REQUIRE(v.type() == j::properties::PROPERTY_TYPE::FLOAT);
//   }

//   {
//     int i = 1;
//     j::properties::access::property_access ia{i};
//     a.assign_from(ia);
//     REQUIRE(v.type() == j::properties::PROPERTY_TYPE::INT);
//   }


//   {
//     j::strings::string s{"frooba"};
//     j::properties::access::property_access sa{s};
//     a.assign_from(sa);
//     REQUIRE(v.type() == j::properties::PROPERTY_TYPE::STRING);
//   }
// }
