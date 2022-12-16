#include "geometry/vec2.hpp"
#include "geometry/vec3.hpp"
#include "geometry/vec4.hpp"
#include "strings/formatting/formatter.hpp"
#include "strings/styling/default_styles.hpp"
#include "strings/styling/styled_sink.hpp"
#include "strings/formatters/numbers.hpp"

namespace j::geometry {
  namespace {
    namespace s = ::j::strings;

    namespace f = s::formatters;

    template<typename Num>
    J_NO_DEBUG J_NO_INLINE void format_vec(const strings::const_string_view & format_options,
                    s::styled_sink & target,
                    s::style current_style,
                    u8_t sz, const Num * nums) {
      const auto & fo = f::number_formatter_v<Num>;
      target.write_styled(s::styles::bold, "(", 1);
      for (u8_t i = 0; i < sz; ++i) {
        if (i != 0) {
          target.write_styled(s::styles::bold, ", ", 2);
        }
        fo.do_format(format_options, nums[i], target, current_style);
      }
      target.write_styled(s::styles::bold, ")", 1);
    }

    template<typename Num>
    [[nodiscard]] J_NO_DEBUG J_NO_INLINE u32_t get_vec_size(const strings::const_string_view & format_options,
                                                            u8_t sz, Num * J_NOT_NULL nums) noexcept {
      u32_t result = sz * 2;
      const auto & fo = f::number_formatter_v<Num>;
      for (u8_t i = 0; i < sz; ++i) {
        result += fo.do_get_length(format_options, nums[i]);
      }
      return result;
    }

    template<typename Num, typename AsNum>
    class J_TYPE_HIDDEN vec2_formatter final : public s::formatter_known_length<vec2<Num>> {
    public:
      void do_format(
        const strings::const_string_view & format_options,
        const vec2<Num> & value,
        s::styled_sink & target,
        s::style current_style
      ) const override {
        format_vec<AsNum>(format_options, target, current_style, 2, (AsNum[]){value.x, value.y});
      }

      [[nodiscard]] u32_t do_get_length(
        const strings::const_string_view & format_options,
        const vec2<Num> & value
      ) const noexcept override {
        return get_vec_size<AsNum>(format_options, 2, (AsNum[]){value.x, value.y});
      }
    };

    template<typename Num, typename AsNum>
    class J_TYPE_HIDDEN vec3_formatter final : public s::formatter_known_length<vec3<Num>> {
    public:
      void do_format(
        const strings::const_string_view & format_options,
        const vec3<Num> & value,
        s::styled_sink & target,
        s::style current_style
      ) const override {
        format_vec<AsNum>(format_options, target, current_style, 3, (AsNum[]){value.x, value.y, value.z});
      }

      [[nodiscard]] u32_t do_get_length(
        const strings::const_string_view & format_options,
        const vec3<Num> & value
      ) const noexcept override {
        return get_vec_size<AsNum>(format_options, 3, (AsNum[]){value.x, value.y, value.z});
      }
    };

    template<typename Num, typename AsNum>
    class J_TYPE_HIDDEN vec4_formatter final : public s::formatter_known_length<vec4<Num>> {
    public:
      void do_format(
        const strings::const_string_view & format_options,
        const vec4<Num> & value,
        s::styled_sink & target,
        s::style current_style
      ) const override {
        format_vec<AsNum>(format_options, target, current_style, 4, (AsNum[]){value.x, value.y, value.z, value.w});
      }

      [[nodiscard]] u32_t do_get_length(
        const strings::const_string_view & format_options,
        const vec4<Num> & value
      ) const noexcept override {
        return get_vec_size<AsNum>(format_options, 4, (AsNum[]){value.x, value.y, value.z, value.w});
      }
    };

    J_A(ND, NODESTROY) const vec2_formatter<float, double> vec2_float_formatter;
    J_A(ND, NODESTROY) const vec2_formatter<u8_t, u64_t> vec2_u8_formatter;
    J_A(ND, NODESTROY) const vec2_formatter<u16_t, u64_t> vec2_u16_formatter;
    J_A(ND, NODESTROY) const vec2_formatter<u32_t, u64_t> vec2_u32_formatter;
    J_A(ND, NODESTROY) const vec2_formatter<u64_t, u64_t> vec2_u64_formatter;
    J_A(ND, NODESTROY) const vec2_formatter<i8_t, i32_t> vec2_i8_formatter;
    J_A(ND, NODESTROY) const vec2_formatter<i16_t, i32_t> vec2_i16_formatter;
    J_A(ND, NODESTROY) const vec2_formatter<i32_t, i32_t> vec2_i32_formatter;

    J_A(ND, NODESTROY) const vec3_formatter<float, double> vec3_float_formatter;
    J_A(ND, NODESTROY) const vec3_formatter<double, double> vec3_double_formatter;
    J_A(ND, NODESTROY) const vec3_formatter<u8_t, u64_t> vec3_u8_formatter;
    J_A(ND, NODESTROY) const vec3_formatter<u16_t, u64_t> vec3_u16_formatter;
    J_A(ND, NODESTROY) const vec3_formatter<u32_t, u64_t> vec3_u32_formatter;
    J_A(ND, NODESTROY) const vec3_formatter<u64_t, u64_t> vec3_u64_formatter;
    J_A(ND, NODESTROY) const vec3_formatter<i8_t, i32_t> vec3_i8_formatter;
    J_A(ND, NODESTROY) const vec3_formatter<i16_t, i32_t> vec3_i16_formatter;
    J_A(ND, NODESTROY) const vec3_formatter<i32_t, i32_t> vec3_i32_formatter;

    J_A(ND, NODESTROY) const vec4_formatter<float, double> vec4_float_formatter;
    J_A(ND, NODESTROY) const vec4_formatter<u8_t, u64_t> vec4_u8_formatter;
    J_A(ND, NODESTROY) const vec4_formatter<u16_t, u64_t> vec4_u16_formatter;
    J_A(ND, NODESTROY) const vec4_formatter<u32_t, u64_t> vec4_u32_formatter;
    J_A(ND, NODESTROY) const vec4_formatter<u64_t, u64_t> vec4_u64_formatter;
    J_A(ND, NODESTROY) const vec4_formatter<i8_t, i32_t> vec4_i8_formatter;
    J_A(ND, NODESTROY) const vec4_formatter<i16_t, i32_t> vec4_i16_formatter;
    J_A(ND, NODESTROY) const vec4_formatter<i32_t, i32_t> vec4_i32_formatter;
  }
}
