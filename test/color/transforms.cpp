#include <detail/preamble.hpp>

#include "colors/srgb.hpp"
#include "colors/xyz.hpp"
#include "colors/lab.hpp"

using namespace j::colors;
namespace g = j::geometry;

namespace {
  template<typename Float>
  void require_rgb_equals(const rgb<Float> & lhs, const rgb<Float> & rhs, Float margin = 0.0002) {
    CHECK  (lhs.r == doctest::Approx(rhs.r).epsilon(margin));
    CHECK  (lhs.g == doctest::Approx(rhs.g).epsilon(margin));
    REQUIRE(lhs.b == doctest::Approx(rhs.b).epsilon(margin));
  }

  template<typename Float>
  void require_xyz_equals(const g::vec3<Float> & lhs, const g::vec3<Float> & rhs, Float margin = 0.0002) {
    CHECK  (lhs.x == doctest::Approx(rhs.x).epsilon(margin));
    CHECK  (lhs.y == doctest::Approx(rhs.y).epsilon(margin));
    REQUIRE(lhs.z == doctest::Approx(rhs.z).epsilon(margin));
  }

  template<typename Float>
  // Note that the margin is larger, because the values are larger.
  void require_lab_equals(const g::vec3<Float> & lhs, const g::vec3<Float> & rhs, Float margin = 0.005) {
    CHECK  (lhs.x == doctest::Approx(rhs.x).epsilon(margin));
    CHECK  (lhs.y == doctest::Approx(rhs.y).epsilon(margin));
    REQUIRE(lhs.z == doctest::Approx(rhs.z).epsilon(margin));
  }

  inline constexpr rgbf black_rgb(0.0, 0.0, 0.0);
  inline constexpr rgbf half_rgb (0.5, 0.5, 0.5);
  inline constexpr rgbf white_rgb(1.0, 1.0, 1.0);
  inline constexpr rgbf different_components_rgb(1.0, 0.5, 0.0);
}

TEST_CASE("srgb to linear") {
  constexpr rgbf cutoff(0.04045, 0.04045, 0.04045);
  require_rgb_equals(srgb_to_linear_rgb(black_rgb),                black_rgb);
  require_rgb_equals(srgb_to_linear_rgb(white_rgb),                white_rgb);
  require_rgb_equals(srgb_to_linear_rgb(half_rgb),                 rgbf(0.214041,    0.214041,    0.214041));
  require_rgb_equals(srgb_to_linear_rgb(cutoff),                   rgbf(0.003130804, 0.003130804, 0.003130804));
  require_rgb_equals(srgb_to_linear_rgb(different_components_rgb), rgbf(1.0,         0.214041,    0));
}

TEST_CASE("linear to srgb") {
  constexpr rgbf cutoff(0.003130804, 0.003130804, 0.003130804);
  require_rgb_equals(linear_rgb_to_srgb(black_rgb),                black_rgb);
  require_rgb_equals(linear_rgb_to_srgb(white_rgb),                white_rgb);
  require_rgb_equals(linear_rgb_to_srgb(half_rgb),                 rgbf(0.735356, 0.735356, 0.735356));
  require_rgb_equals(linear_rgb_to_srgb(cutoff),                   rgbf(0.04045,  0.04045,  0.04045));
  require_rgb_equals(linear_rgb_to_srgb(different_components_rgb), rgbf(1.0,      0.735356, 0.0));
}

TEST_CASE("srgb to xyz") {
  require_xyz_equals(srgb_to_xyz_d65(black_rgb),                xyzf(0,       0,       0));
  require_xyz_equals(srgb_to_xyz_d65(white_rgb),                xyzf(0.95047, 1.0,     1.08883));
  require_xyz_equals(srgb_to_xyz_d65(half_rgb),                 xyzf(0.20344, 0.21404, 0.23305));
  require_xyz_equals(srgb_to_xyz_d65(different_components_rgb), xyzf(0.48899, 0.36574, 0.04485));
}

TEST_CASE("xyz to srgb") {
  require_rgb_equals(xyz_d65_to_srgb(xyzf(0.0f, 0.0f, 0.0f)), black_rgb);
  // Note that red is clamped here:
  require_rgb_equals(xyz_d65_to_srgb(xyzf(1.0f, 1.0f, 1.0f)), rgbf(1.0, 0.97692, 0.95881));
  require_rgb_equals(xyz_d65_to_srgb(xyzf(0.5f, 0.5f, 0.5f)), rgbf(0.79915, 0.71807, 0.70450));
  // Red clamped again:
  require_rgb_equals(xyz_d65_to_srgb(xyzf(0.0f, 0.5f, 1.0f)), rgbf(0.0, 0.99096, 0.98005));
}

TEST_CASE("srgb to lab") {
  require_lab_equals(srgb_to_lab(black_rgb),                labf(0,      0,      0));
  require_lab_equals(srgb_to_lab(white_rgb),                labf(100.0,  0.0,    0.0));
  require_lab_equals(srgb_to_lab(half_rgb),                 labf(53.389, 0.0,    0.0));
  require_lab_equals(srgb_to_lab(different_components_rgb), labf(66.957, 43.072, 73.959));
}

TEST_CASE("lab to srgb") {
  require_rgb_equals(lab_to_srgb(labf(0.0f, 0.0f, 0.0f)), black_rgb);
  // Red is clamped:
  require_rgb_equals(lab_to_srgb(labf(100.f, 100.f, 100.f)), rgbf(1.0, 0.57463, 0.19397));
  require_rgb_equals(lab_to_srgb(labf(50.f, -50.f, 50.f)), rgbf(0.13783, 0.53905, 0.06322));
  // Green is clamped:
  require_rgb_equals(lab_to_srgb(labf(40.f, 91.f, -20.f)), rgbf(0.81202, 0.0, 0.50681));
}
