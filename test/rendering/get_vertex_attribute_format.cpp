// #include <detail/preamble.hpp>

// #include "colors/rgb.hpp"
// #include "geometry/rect.hpp"
// #include "geometry/vec2.hpp"
// #include "geometry/vec4.hpp"
// #include "rendering/vertex_data/get_vertex_attribute_format.hpp"

// namespace dt = ::j::rendering::data_types;
// namespace v = ::j::rendering::vertex_data;
// namespace c = ::j::colors;
// namespace g = ::j::geometry;

// TEST_SUITE("Get Vertex Attribute Format") {
//   TEST_CASE("Scalar types") {
//     REQUIRE(v::get_vertex_attribute_format<u8_t> ()                    == v::vertex_attribute_format::u8_normalized);
//     REQUIRE(v::get_vertex_attribute_format<u16_t>()                    == v::vertex_attribute_format::u16_normalized);
//     REQUIRE(v::get_vertex_attribute_format<u32_t>()                    == v::vertex_attribute_format::u32_normalized);
//     REQUIRE(v::get_vertex_attribute_format<i8_t> ()                    == v::vertex_attribute_format::s8_normalized);
//     REQUIRE(v::get_vertex_attribute_format<i16_t>()                    == v::vertex_attribute_format::s16_normalized);
//     REQUIRE(v::get_vertex_attribute_format<i32_t>()                    == v::vertex_attribute_format::s32_normalized);
//     REQUIRE(v::get_vertex_attribute_format<float>()                    == v::vertex_attribute_format::single_float);

//     REQUIRE(v::get_vertex_attribute_format<u8_t> (v::scaling::scaled)  == v::vertex_attribute_format::u8_scaled);
//     REQUIRE(v::get_vertex_attribute_format<u16_t>(v::scaling::scaled)  == v::vertex_attribute_format::u16_scaled);
//     REQUIRE(v::get_vertex_attribute_format<u32_t>(v::scaling::scaled)  == v::vertex_attribute_format::u32_scaled);
//     REQUIRE(v::get_vertex_attribute_format<i8_t> (v::scaling::scaled)  == v::vertex_attribute_format::s8_scaled);
//     REQUIRE(v::get_vertex_attribute_format<i16_t>(v::scaling::scaled)  == v::vertex_attribute_format::s16_scaled);
//     REQUIRE(v::get_vertex_attribute_format<i32_t>(v::scaling::scaled)  == v::vertex_attribute_format::s32_scaled);

//     REQUIRE(v::get_vertex_attribute_format<u8_t> (v::scaling::integer) == v::vertex_attribute_format::u8_int);
//     REQUIRE(v::get_vertex_attribute_format<u16_t>(v::scaling::integer) == v::vertex_attribute_format::u16_int);
//     REQUIRE(v::get_vertex_attribute_format<u32_t>(v::scaling::integer) == v::vertex_attribute_format::u32_int);
//     REQUIRE(v::get_vertex_attribute_format<i8_t> (v::scaling::integer) == v::vertex_attribute_format::s8_int);
//     REQUIRE(v::get_vertex_attribute_format<i16_t>(v::scaling::integer) == v::vertex_attribute_format::s16_int);
//     REQUIRE(v::get_vertex_attribute_format<i32_t>(v::scaling::integer) == v::vertex_attribute_format::s32_int);
//   }

//   TEST_CASE("Vec2 types") {
//     REQUIRE(v::get_vertex_attribute_format<g::vec2u8>()      == v::vertex_attribute_format::u8_vec2_normalized);
//     REQUIRE(v::get_vertex_attribute_format<g::vec2u16>()     == v::vertex_attribute_format::u16_vec2_normalized);
//     REQUIRE(v::get_vertex_attribute_format<g::vec2u32>()     == v::vertex_attribute_format::u32_vec2_normalized);
//     REQUIRE(v::get_vertex_attribute_format<g::vec2i8>()      == v::vertex_attribute_format::s8_vec2_normalized);
//     REQUIRE(v::get_vertex_attribute_format<g::vec2i16>()     == v::vertex_attribute_format::s16_vec2_normalized);
//     REQUIRE(v::get_vertex_attribute_format<g::vec2i32>()     == v::vertex_attribute_format::s32_vec2_normalized);
//     REQUIRE(v::get_vertex_attribute_format<g::vec2f>()       == v::vertex_attribute_format::single_float_vec2);
//   }

//   TEST_CASE("Vec4 types") {
//     REQUIRE(v::get_vertex_attribute_format<g::vec4u8>()      == v::vertex_attribute_format::u8_vec4_normalized);
//     REQUIRE(v::get_vertex_attribute_format<g::vec4u16>()     == v::vertex_attribute_format::u16_vec4_normalized);
//     REQUIRE(v::get_vertex_attribute_format<g::vec4u32>()     == v::vertex_attribute_format::u32_vec4_normalized);
//     REQUIRE(v::get_vertex_attribute_format<g::vec4i8>()      == v::vertex_attribute_format::s8_vec4_normalized);
//     REQUIRE(v::get_vertex_attribute_format<g::vec4i16>()     == v::vertex_attribute_format::s16_vec4_normalized);
//     REQUIRE(v::get_vertex_attribute_format<g::vec4i32>()     == v::vertex_attribute_format::s32_vec4_normalized);
//     REQUIRE(v::get_vertex_attribute_format<g::vec4f>()       == v::vertex_attribute_format::single_float_vec4);
//   }

//   TEST_CASE("RGB types") {
//     REQUIRE(v::get_vertex_attribute_format<c::rgb8>()        == v::vertex_attribute_format::u8_vec3_normalized);
//     REQUIRE(v::get_vertex_attribute_format<c::rgbf>()        == v::vertex_attribute_format::single_float_vec3);
//     REQUIRE(v::get_vertex_attribute_format<c::rgb<u16_t>>()  == v::vertex_attribute_format::u16_vec3_normalized);
//     REQUIRE(v::get_vertex_attribute_format<c::rgb<u32_t>>()  == v::vertex_attribute_format::u32_vec3_normalized);
//     REQUIRE(v::get_vertex_attribute_format<c::rgb<i8_t>>()   == v::vertex_attribute_format::s8_vec3_normalized);
//     REQUIRE(v::get_vertex_attribute_format<c::rgb<i16_t>>()  == v::vertex_attribute_format::s16_vec3_normalized);
//     REQUIRE(v::get_vertex_attribute_format<c::rgb<i32_t>>()  == v::vertex_attribute_format::s32_vec3_normalized);
//   }

//   TEST_CASE("RGBA types") {
//     REQUIRE(v::get_vertex_attribute_format<c::rgba8>()       == v::vertex_attribute_format::u8_vec4_normalized);
//     REQUIRE(v::get_vertex_attribute_format<c::rgbaf>()       == v::vertex_attribute_format::single_float_vec4);
//     REQUIRE(v::get_vertex_attribute_format<c::rgba<u16_t>>() == v::vertex_attribute_format::u16_vec4_normalized);
//     REQUIRE(v::get_vertex_attribute_format<c::rgba<u32_t>>() == v::vertex_attribute_format::u32_vec4_normalized);
//     REQUIRE(v::get_vertex_attribute_format<c::rgba<i8_t>>()  == v::vertex_attribute_format::s8_vec4_normalized);
//     REQUIRE(v::get_vertex_attribute_format<c::rgba<i16_t>>() == v::vertex_attribute_format::s16_vec4_normalized);
//     REQUIRE(v::get_vertex_attribute_format<c::rgba<i32_t>>() == v::vertex_attribute_format::s32_vec4_normalized);
//   }

//   TEST_CASE("Rectangle types") {
//     REQUIRE(v::get_vertex_attribute_format<g::rect_u16>()   == v::vertex_attribute_format::u16_vec4_normalized);
//     REQUIRE(v::get_vertex_attribute_format<g::rect_u32>()   == v::vertex_attribute_format::u32_vec4_normalized);
//     REQUIRE(v::get_vertex_attribute_format<g::rect_i16>()   == v::vertex_attribute_format::s16_vec4_normalized);
//     REQUIRE(v::get_vertex_attribute_format<g::rect_i32>()   == v::vertex_attribute_format::s32_vec4_normalized);
//     REQUIRE(v::get_vertex_attribute_format<g::rect_float>() == v::vertex_attribute_format::single_float_vec4);
//   }

//   TEST_CASE("Aligned sizes") {
//     constexpr v::vertex_attribute_format one_byte_formats[] = {
//           v::vertex_attribute_format::u8_normalized, v::vertex_attribute_format::s8_normalized,
//           v::vertex_attribute_format::u8_scaled, v::vertex_attribute_format::s8_scaled,
//           v::vertex_attribute_format::u8_int, v::vertex_attribute_format::s8_int,
//     };

//     constexpr v::vertex_attribute_format two_byte_formats[] = {
//           v::vertex_attribute_format::u8_vec2_normalized, v::vertex_attribute_format::s8_vec2_normalized,
//           v::vertex_attribute_format::u8_vec2_scaled, v::vertex_attribute_format::s8_vec2_scaled,
//           v::vertex_attribute_format::u8_vec2_int, v::vertex_attribute_format::s8_vec2_int,
//           v::vertex_attribute_format::u16_normalized, v::vertex_attribute_format::s16_normalized,
//           v::vertex_attribute_format::u16_scaled, v::vertex_attribute_format::s16_scaled,
//           v::vertex_attribute_format::u16_int, v::vertex_attribute_format::s16_int,
//     };
//     constexpr v::vertex_attribute_format four_byte_formats[] = {
//           v::vertex_attribute_format::u8_vec4_normalized, v::vertex_attribute_format::s8_vec4_normalized,
//           v::vertex_attribute_format::u8_vec4_scaled, v::vertex_attribute_format::s8_vec4_scaled,
//           v::vertex_attribute_format::u8_vec4_int, v::vertex_attribute_format::s8_vec4_int,
//           v::vertex_attribute_format::u8_vec3_normalized, v::vertex_attribute_format::s8_vec3_normalized,
//           v::vertex_attribute_format::u8_vec3_scaled, v::vertex_attribute_format::s8_vec3_scaled,
//           v::vertex_attribute_format::u8_vec3_int, v::vertex_attribute_format::s8_vec3_int,
//           v::vertex_attribute_format::u16_vec2_normalized, v::vertex_attribute_format::s16_vec2_normalized,
//           v::vertex_attribute_format::u16_vec2_scaled, v::vertex_attribute_format::s16_vec2_scaled,
//           v::vertex_attribute_format::u16_vec2_int, v::vertex_attribute_format::s16_vec2_int,
//           v::vertex_attribute_format::u32_normalized, v::vertex_attribute_format::s32_normalized,
//           v::vertex_attribute_format::u32_scaled, v::vertex_attribute_format::s32_scaled,
//           v::vertex_attribute_format::u32_int, v::vertex_attribute_format::s32_int,
//           v::vertex_attribute_format::single_float,
//     };

//     constexpr v::vertex_attribute_format eight_byte_formats[] = {
//           v::vertex_attribute_format::u16_vec3_normalized, v::vertex_attribute_format::s16_vec3_normalized,
//           v::vertex_attribute_format::u16_vec3_scaled, v::vertex_attribute_format::s16_vec3_scaled,
//           v::vertex_attribute_format::u16_vec3_int, v::vertex_attribute_format::s16_vec3_int,
//           v::vertex_attribute_format::u16_vec4_normalized, v::vertex_attribute_format::s16_vec4_normalized,
//           v::vertex_attribute_format::u16_vec4_scaled, v::vertex_attribute_format::s16_vec4_scaled,
//           v::vertex_attribute_format::u16_vec4_int, v::vertex_attribute_format::s16_vec4_int,
//           v::vertex_attribute_format::u32_vec2_normalized, v::vertex_attribute_format::s32_vec2_normalized,
//           v::vertex_attribute_format::u32_vec2_scaled, v::vertex_attribute_format::s32_vec2_scaled,
//           v::vertex_attribute_format::u32_vec2_int, v::vertex_attribute_format::s32_vec2_int,
//           v::vertex_attribute_format::single_float_vec2,
//     };

//     constexpr v::vertex_attribute_format sixteen_byte_formats[] = {
//           v::vertex_attribute_format::u32_vec3_normalized, v::vertex_attribute_format::s32_vec3_normalized,
//           v::vertex_attribute_format::u32_vec3_scaled, v::vertex_attribute_format::s32_vec3_scaled,
//           v::vertex_attribute_format::u32_vec3_int, v::vertex_attribute_format::s32_vec3_int,
//           v::vertex_attribute_format::u32_vec4_normalized, v::vertex_attribute_format::s32_vec4_normalized,
//           v::vertex_attribute_format::u32_vec4_scaled, v::vertex_attribute_format::s32_vec4_scaled,
//           v::vertex_attribute_format::u32_vec4_int, v::vertex_attribute_format::s32_vec4_int,
//           v::vertex_attribute_format::single_float_vec3,
//           v::vertex_attribute_format::single_float_vec4,
//     };
//     for (auto f : one_byte_formats)     { REQUIRE(dt::data_type_size(f)  == 1U); }
//     for (auto f : two_byte_formats)     { REQUIRE(dt::data_type_size(f)  == 2U); }
//     for (auto f : four_byte_formats)    { REQUIRE(dt::data_type_size(f)  == 4U); }
//     for (auto f : eight_byte_formats)   { REQUIRE(dt::data_type_size(f)  == 8U); }
//     for (auto f : sixteen_byte_formats) { REQUIRE(dt::data_type_size(f)  == 16U); }
//   }
// }

// static void require_single(v::vertex_data_info i, v::vertex_attribute_format f) {
//   REQUIRE(i.size() == 1);
//   REQUIRE(i[0] == f);
//   REQUIRE(i.vertex_size_bytes() == 1U);
// }

// TEST_SUITE("Get Vertex Attribute Info") {
//   struct J_TYPE_HIDDEN single {
//     u8_t value;
//   };

//   TEST_CASE("Single attribute") {
//     require_single(v::get_single_vertex_data_info<u8_t>(),                       v::vertex_attribute_format::u8_normalized);
//     require_single(v::get_single_vertex_data_info<u8_t>(v::scaling::normalized), v::vertex_attribute_format::u8_normalized);
//     require_single(v::get_single_vertex_data_info<u8_t>(v::scaling::scaled),     v::vertex_attribute_format::u8_scaled);
//     require_single(v::get_single_vertex_data_info<u8_t>(v::scaling::integer),    v::vertex_attribute_format::u8_int);
//     require_single(v::get_vertex_data_info(&single::value),                      v::vertex_attribute_format::u8_normalized);
//     require_single(v::get_vertex_data_info(v::normalized(&single::value)),       v::vertex_attribute_format::u8_normalized);
//     require_single(v::get_vertex_data_info(v::scaled(&single::value)),           v::vertex_attribute_format::u8_scaled);
//     require_single(v::get_vertex_data_info(v::integer(&single::value)),          v::vertex_attribute_format::u8_int);
//   }

//   struct J_TYPE_HIDDEN two_vectors {
//     c::rgbf a;
//     alignas(16) g::vec2u8 b;
//   };
//   TEST_CASE("Two vectors") {
//     auto i = v::get_vertex_data_info(&two_vectors::a, &two_vectors::b);
//     REQUIRE(i.size() == 2);
//     REQUIRE(i.vertex_size_bytes() == 18U);
//     REQUIRE(i[0] == v::vertex_attribute_format::single_float_vec3);
//     REQUIRE(i[1] == v::vertex_attribute_format::u8_vec2_normalized);
//   }
// }
