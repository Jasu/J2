#include <detail/preamble.hpp>

#include "properties/property_variant.hpp"
#include "containers/unsorted_string_map.hpp"
#include "containers/vector.hpp"

namespace {
  void test_non_empty_invariants(const j::properties::property_variant & v) {
    REQUIRE(!v.empty());
    REQUIRE(v == v);
    REQUIRE(!(v != v));
  }

  void test_type(const j::properties::property_variant & v, j::properties::PROPERTY_TYPE type) {
    test_non_empty_invariants(v);
    REQUIRE(v.type() == type);
    REQUIRE(v.is(type));
  }
  template<typename T>
  void test_non_empty(j::properties::property_variant & v, j::properties::PROPERTY_TYPE type, T value) {
    test_type(v, type);
    REQUIRE(v.as<T>() == value);

    // Copy-construction works
    {
      j::properties::property_variant v2{v};
      test_type(v2, type);
      REQUIRE(v2.as<T>() == value);
      REQUIRE(v2 == v);
    }

    // Copy-assignment works
    {
      j::properties::property_variant v2;
      REQUIRE(&v2 == &(v2 = v));
      test_type(v2, type);
      REQUIRE(v2.as<T>() == value);
      REQUIRE(v2 == v);
    }

    // Self-assign doesn't break anything
    {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wself-assign-overloaded"
      REQUIRE(&v == &(v = v));
#pragma clang diagnostic pop
      test_type(v, type);
      REQUIRE(v.as<T>() == value);
    }

    // Self-move doesn't break anything
    {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wself-move"
      REQUIRE(&v == &(v = static_cast<j::properties::property_variant &&>(v)));
#pragma clang diagnostic pop
      test_type(v, type);
      REQUIRE(v.as<T>() == value);
    }

    // Move-construction works
    {
      j::properties::property_variant v2{v};
      j::properties::property_variant v3{static_cast<j::properties::property_variant &&>(v2)};
      REQUIRE(v2.empty());
      test_type(v3, type);
      REQUIRE(v3.as<T>() == value);
      REQUIRE(v3 == v);
    }

    // Move-assignment works
    {
      j::properties::property_variant v2{v};
      j::properties::property_variant v3;
      REQUIRE(&v3 == &(v3 = static_cast<j::properties::property_variant &&>(v2)));
      REQUIRE(v2.empty());
      test_type(v3, type);
      REQUIRE(v3.as<T>() == value);
      REQUIRE(v3 == v);
    }
  }
}

TEST_CASE("Empty property variant basics") {
  j::properties::property_variant v;
  REQUIRE(v.empty());
  REQUIRE(v.type() == j::properties::PROPERTY_TYPE::EMPTY);

  {
    j::properties::property_variant v2{v};
    REQUIRE(v2.empty());
    REQUIRE(v2.type() == j::properties::PROPERTY_TYPE::EMPTY);
  }

  {
    j::properties::property_variant v3;
    v3 = v;
    REQUIRE(v3.empty());
    REQUIRE(v3.type() == j::properties::PROPERTY_TYPE::EMPTY);
  }

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wself-assign-overloaded"
  v = v;
#pragma clang diagnostic pop
  REQUIRE(v.empty());
  REQUIRE(v.type() == j::properties::PROPERTY_TYPE::EMPTY);

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wself-move"
  v = static_cast<j::properties::property_variant &&>(v);
#pragma clang diagnostic pop
  REQUIRE(v.empty());
  REQUIRE(v.type() == j::properties::PROPERTY_TYPE::EMPTY);

  REQUIRE(v == v);
}

TEST_CASE("Int property variant basics") {
  j::properties::property_variant v = 2;
  test_non_empty<j::properties::int_t>(v, j::properties::PROPERTY_TYPE::INT, 2);
}

TEST_CASE("Float property variant basics") {
  j::properties::property_variant v = 2.0;
  test_non_empty<j::properties::float_t>(v, j::properties::PROPERTY_TYPE::FLOAT, 2.0);
}

TEST_CASE("Bool property variant basics") {
  {
    j::properties::property_variant v = false;
    test_non_empty<j::properties::bool_t>(v, j::properties::PROPERTY_TYPE::BOOL, false);
  }

  {
    j::properties::property_variant v = true;
    test_non_empty<j::properties::bool_t>(v, j::properties::PROPERTY_TYPE::BOOL, true);
  }
}

TEST_CASE("Nil property variant basics") {
    j::properties::property_variant v = nullptr;
    test_non_empty<j::properties::nil_t>(v, j::properties::PROPERTY_TYPE::NIL, nullptr);
}

TEST_CASE("List property variant basics") {
  j::properties::property_variant v = j::properties::list_t{};
  test_non_empty<j::properties::list_t>(v, j::properties::PROPERTY_TYPE::LIST, j::properties::list_t{});
}

TEST_CASE("Map property variant basics") {
  j::properties::property_variant v = j::properties::map_t{};
  test_non_empty<j::properties::map_t>(v, j::properties::PROPERTY_TYPE::MAP, j::properties::map_t{});
}

inline void test_assign_to_empty(const j::properties::property_variant & rhs) {
  j::properties::property_variant target;
  {
    REQUIRE(target.empty());
    target = rhs;
    REQUIRE(!target.empty());
    REQUIRE(target.type() == rhs.type());
    REQUIRE(target == rhs);
  }

  {
    j::properties::property_variant target;
    j::properties::property_variant src{rhs};
    REQUIRE(target.empty());
    target = static_cast<j::properties::property_variant &&>(src);
    REQUIRE(src.empty());
    REQUIRE(!target.empty());
    REQUIRE(target.type() == rhs.type());
    REQUIRE(target == rhs);

  }
}

TEST_CASE("Property variant assignment to empty value") {
  j::properties::property_variant int_variant = 2;
  j::properties::property_variant float_variant = 2.0;
  j::properties::property_variant bool_variant = false;
  j::properties::property_variant nil_variant = nullptr;
  j::properties::property_variant map_variant = j::properties::map_t{};
  j::properties::property_variant list_variant = j::properties::list_t{};

  test_assign_to_empty(int_variant);
  test_assign_to_empty(float_variant);
  test_assign_to_empty(bool_variant);
  test_assign_to_empty(nil_variant);
  test_assign_to_empty(map_variant);
  test_assign_to_empty(list_variant);
}

TEST_CASE("Property variant type-changing assignment") {
  j::properties::property_variant variants[] = { 2, 2.0, false, nullptr, j::properties::map_t{}, j::properties::list_t{} };
  for (sz_t i = 0; i < 6; ++i) {
    for (sz_t j = 0; j < 6; ++j) {
      if (i == j) {
        continue;
      }
      {
        j::properties::property_variant target = variants[i];
        j::properties::property_variant src = variants[j];
        REQUIRE(target.type() != src.type());
        REQUIRE(target != src);
        target = src;
        REQUIRE(target.type() == src.type());
        REQUIRE(target == src);
      }

      {
        j::properties::property_variant target = variants[i];
        j::properties::property_variant src = variants[j];
        REQUIRE(target.type() != src.type());
        REQUIRE(target != src);
        target = static_cast<j::properties::property_variant &&>(src);
        REQUIRE(src.empty());
        REQUIRE(target.type() == variants[j].type());
        REQUIRE(target == variants[j]);
      }
    }
  }
}
