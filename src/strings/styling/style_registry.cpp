#include "strings/styling/style_registry.hpp"

#include "containers/unsorted_string_map.hpp"
#include "colors/default_colors.hpp"
#include "strings/styling/default_styles.hpp"
#include "strings/string_map.hpp"

J_DEFINE_EXTERN_STRING_MAP(j::strings::styling::style);

template struct j::util::singleton<j::strings::styling::style_registry>;

namespace j::strings::inline styling {
  using color = colors::color;
  namespace c = colors::colors;
  namespace styles {
    J_A(ND) const style bold            {styling::bold};

    J_A(ND) const style emergency       {c::bright_red,    c::gray,           styling::bold};
    J_A(ND) const style alert           {c::bright_red,    c::gray,           styling::bold};
    J_A(ND) const style critical        {c::bright_red,    c::gray,           styling::bold};
    J_A(ND) const style error           {c::bright_yellow, c::bright_red,     styling::bold};
    J_A(ND) const style warning         {c::black,         c::bright_yellow,  styling::bold};
    J_A(ND) const style notice          {c::black,         c::bright_magenta, styling::bold};
    J_A(ND) const style info            {c::white,         c::bright_blue,    styling::bold};
    J_A(ND) const style debug           {c::white,         c::cyan,           styling::bold};

    J_A(ND) const style white_on_gray   {c::white, c::gray, styling::bold};
    J_A(ND) const style yellow_on_red   {c::bright_yellow, c::red, styling::bold};
    J_A(ND) const style black_on_yellow {c::black, c::bright_yellow};

    J_A(ND) const style tag1            {c::white,          c::green,          styling::bold};
    J_A(ND) const style tag2            {c::white,          c::bright_blue,    styling::bold};
    J_A(ND) const style tag3            {c::black,          c::bright_green,   styling::bold};
    J_A(ND) const style tag4            {c::white,          c::cyan,           styling::bold};
    J_A(ND) const style tag5            {c::white,          c::bright_magenta, styling::bold};
    J_A(ND) const style tag6            {c::black,          c::light_gray,     styling::bold};
    J_A(ND) const style tag7            {c::white,          c::magenta,        styling::bold};
    J_A(ND) const style tag8            {c::black,          c::bright_cyan,    styling::bold};
    J_A(ND) const style tag9            {c::black,          c::white,          styling::bold};
    J_A(ND) const style tag10           {c::bright_yellow,  c::green,          styling::bold};
    J_A(ND) const style tag11           {c::bright_yellow,  c::blue,           styling::bold};
    J_A(ND) const style tag12           {c::bright_yellow,  c::magenta,        styling::bold};
    J_A(ND) const style tag13           {c::bright_magenta, c::gray,           styling::bold};
    J_A(ND) const style tag14           {c::bright_magenta, c::green,          styling::bold};
    J_A(ND) const style tag15           {c::white,          c::gray,           styling::bold};
    J_A(ND) const style tag16           {c::bright_cyan,    c::magenta,        styling::bold};

    J_A(ND) const style bright_red      {c::bright_red};
    J_A(ND) const style bright_green    {c::bright_green};
    J_A(ND) const style bright_yellow   {c::bright_yellow};
    J_A(ND) const style bright_blue     {c::bright_blue};
    J_A(ND) const style bright_magenta  {c::bright_magenta};
    J_A(ND) const style bright_cyan     {c::bright_cyan};
    J_A(ND) const style black           {c::black};
    J_A(ND) const style red             {c::red};
    J_A(ND) const style green           {c::green};
    J_A(ND) const style yellow          {c::yellow};
    J_A(ND) const style blue            {c::blue};
    J_A(ND) const style magenta         {c::magenta};
    J_A(ND) const style cyan            {c::cyan};
    J_A(ND) const style light_gray      {c::light_gray};
    J_A(ND) const style gray            {c::gray};
    J_A(ND) const style white           {c::white};
    J_A(ND) const style bold_white      {c::white, styling::bold};
    J_A(ND) const style bold_bright_red {c::bright_red, styling::bold};
  }

  style_registry::style_registry() {
    const pair<const char *, style> styles[]{
      {"bold",                       styles::bold},

      {"debug",                      styles::debug},
      {"info",                       styles::info},
      {"notice",                     styles::notice},
      {"warning",                    styles::warning},
      {"error",                      styles::error},



      {"bright_red",                 styles::bright_red},
      {"bright_green",               styles::bright_green},
      {"bright_yellow",              styles::bright_yellow},
      {"bright_blue",                styles::bright_blue},
      {"bright_magenta",             styles::bright_magenta},
      {"bright_cyan",                styles::bright_cyan},
      {"black",                      styles::black},
      {"red",                        styles::red},
      {"green",                      styles::green},
      {"yellow",                     styles::yellow},
      {"blue",                       styles::blue},
      {"magenta",                    styles::magenta},
      {"cyan",                       styles::cyan},
      {"light_gray",                 styles::light_gray},

      {"gray",                       styles::gray},
      {"white",                      styles::white},

      {"red_bg",                     style{color(), c::red}},
      {"green_bg",                   style{color(), c::green}},
      {"yellow_bg",                  style{color(), c::yellow}},
      {"blue_bg",                    style{color(), c::blue}},
      {"magenta_bg",                 style{color(), c::magenta}},
      {"cyan_bg",                    style{color(), c::cyan}},
      {"light_gray_bg",              style{color(), c::light_gray}},

      {"bright_red_bg",              style{color(), c::bright_red}},
      {"bright_green_bg",            style{color(), c::bright_green}},
      {"bright_yellow_bg",           style{color(), c::bright_yellow}},
      {"bright_blue_bg",             style{color(), c::bright_blue}},
      {"bright_magenta_bg",          style{color(), c::bright_magenta}},
      {"bright_cyan_bg",             style{color(), c::bright_cyan}},
      {"white_bg",                   style{color(), c::white}},
      {"gray_bg",                    style{color(), c::gray}},

      {"black_bg",                   style{color(), c::black}},

      {"underline",                  style{styling::underline}},
      {"strikethrough",              style{styling::strikethrough}},
      {"italic",                     style{styling::italic}},
      {"roman",                      style{styling::no_italic}},
      {"regular",                    style{styling::no_bold}},
      {"no_strikethrough",           style{styling::no_strikethrough}},
      {"no_underline",               style{styling::no_underline}},


      {"critical",                   styles::critical},
      {"alert",                      styles::alert},
      {"emergency",                  styles::emergency},

      {"tag1",                       styles::tag1},
      {"tag2",                       styles::tag2},
      {"tag3",                       styles::tag3},
      {"tag4",                       styles::tag4},
      {"tag5",                       styles::tag5},
      {"tag6",                       styles::tag6},
      {"tag7",                       styles::tag7},
      {"tag8",                       styles::tag8},

      {"white_on_gray",              styles::white_on_gray},
      {"yellow_on_red",              styles::yellow_on_red},
      {"black_on_yellow",            styles::black_on_yellow},
      {"bold_white",                 styles::bold_white},

      {"bright_orange",              style{c::bright_orange}},
      {"orange",                     style{c::orange}},
    };

    for (auto & p : styles) {
      m_styles.emplace((strings::string)p.first, p.second);
    }
  }

  void style_registry::register_style(string && str, const style & style) {
    bool did_register = m_styles.emplace(static_cast<string &&>(str), style).second;
    J_REQUIRE(did_register, "Tried to register style twice.");
  }

  void style_registry::register_style(const const_string_view & str, const style & style) {
    bool did_register = m_styles.emplace(string(str), style).second;
    J_REQUIRE(did_register, "Tried to register style twice.");
  }

  J_A(NODESTROY) constinit util::singleton<style_registry> g_style_registry;
}
