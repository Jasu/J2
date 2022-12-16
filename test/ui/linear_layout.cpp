#include <detail/stringify_geometry.hpp>

#include "ui/layout/linear_layout.hpp"
#include "mem/shared_ptr.hpp"

namespace u = j::ui;
namespace a = j::ui::attributes;
namespace g = j::geometry;
namespace c = j::colors;
namespace m = j::mem;

TEST_SUITE_BEGIN("UI - Linear Layout");

namespace {
  class dummy_element final : public u::static_element {
  public:
    using u::static_element::static_element;
    void render(u::context &, const g::rect_u16 &) const noexcept override {
    }
  };
}

TEST_CASE("Empty") {
  g::sides sides;
  SUBCASE("No sides") { }
  SUBCASE("All sides") { sides = g::all_sides; }
  u::linear_layout l{a::collapse_margins = sides};
  REQUIRE(l.min_size() == u::size{});

  u::linear_layout l2{a::collapse_margins = sides, a::min_size = g::vec2u16{11,93}};
  REQUIRE(l2.min_size() == u::size{ { 11U, 0U, 0U, 0U, 0U }, { 93U, 0U, 0U, 0U, 0U } });


  u::linear_layout l3{
    a::collapse_margins = sides,
    a::min_size = g::vec2u16{0,100},
    a::margin = g::perimeter_u8{1U, 4U, 16U, 64U},
  };
  REQUIRE(l3.min_size() == u::size{ { 0U, 0U, 1U, 0U, 16U }, { 100U, 0U, 4U, 0U, 64U } });

  u::linear_layout l4{
    a::collapse_margins = sides,
    a::min_size = g::vec2u16{50U,100U},
    a::border_size = g::perimeter_u8{1U, 2U, 3U, 4U},
    a::margin = g::perimeter_u8{5U, 6U, 7U, 8U},
  };
  REQUIRE(l4.min_size() == u::size{ { 50U, 1U, 5U, 3U, 7U }, { 100U, 2U, 6U, 4U, 8U } });
}

TEST_CASE("Single object") {
  auto e = m::make_shared<dummy_element>(
    a::min_size = g::vec2u16(100,200),
    a::margin = g::perimeter_u8(1U, 2U, 4U, 8U));

  SUBCASE("All sides") {
    u::linear_layout l{a::collapse_margins = g::all_sides};
    l.push_back(e);
    REQUIRE(l.min_size() == u::size{ { 100, 0U, 1U, 0U, 4U }, { 200U, 0U, 2U, 0U, 8U } });
  }
  SUBCASE("No sides") {
    u::linear_layout l;
    l.push_back(e);
    REQUIRE(l.min_size() == u::size{ { 105, 0U, 0U, 0U, 0U }, { 210U, 0U, 0U, 0U, 0U } });
  }
}

TEST_CASE("Two objects") {
  auto e = m::make_shared<dummy_element>(
                a::min_size = g::vec2u16(100,200),
                a::margin = g::perimeter_u8(1U, 2U, 4U, 8U)
    );
  SUBCASE("All sides") {
    {
      u::linear_layout l(a::collapse_margins = g::all_sides);
      l.push_back(e);
      l.push_back(e);
      REQUIRE(l.min_size() == u::size{ { 100U, 0U, 1U, 0U, 4U }, { 408U, 0U, 2U, 0U, 8U } });
    }
    {
      u::linear_layout l(a::orientation = u::orientation::columns, a::collapse_margins = g::all_sides);
      l.push_back(e);
      l.push_back(e);
      REQUIRE(l.min_size() == u::size{ { 204U, 0U, 1U, 0U, 4U }, { 200U, 0U, 2U, 0U, 8U } });
    }
  }
  SUBCASE("No sides") {
    {
      u::linear_layout l;
      l.push_back(e);
      l.push_back(e);
      REQUIRE(l.min_size() == u::size{ { 105U, 0U, 0U, 0U, 0U }, { 418U, 0U, 0U, 0U, 0U } });
    }
    {
      u::linear_layout l(a::orientation = u::orientation::columns);
      l.push_back(e);
      l.push_back(e);
      REQUIRE(l.min_size() == u::size{ { 209U, 0U, 0U, 0U, 0U }, { 210U, 0U, 0U, 0U, 0U } });
    }
  }
}

TEST_SUITE_END();
