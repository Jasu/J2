#pragma once

#include "strings/styling/style.hpp"
#include "strings/string_map_fwd.hpp"
#include "util/singleton.hpp"

J_DECLARE_EXTERN_STRING_MAP(j::strings::styling::style);

namespace j::strings::inline styling::styles {
  J_A(ND) extern const style bold;
  J_A(ND) extern const style light_gray;
  J_A(ND) extern const style bright_cyan;
  J_A(ND) extern const style bright_blue;
  J_A(ND) extern const style bright_green;
  J_A(ND) extern const style bright_yellow;
  J_A(ND) extern const style bright_magenta;
}

namespace j::strings::inline styling {
  class style_registry final {
  public:
    style_registry();

    void register_style(strings::string && str, const style & style);
    void register_style(const strings::const_string_view & str, const style & style);

    J_A(AI,NODISC) inline const style & at(const_string_view str) const {
      const char *data = str.data();
      u32_t sz = str.size();
      switch (sz) {
      case 4:
        if (!j::memcmp(data, "bold", 4)) {
          return styles::bold;
        }
        break;
      case 10:
        if (!j::memcmp(data, "light_gray", 10)) {
          return styles::light_gray;
        }
        break;
      default: {
        if (sz > 10 && !j::memcmp(data, "bright_", 6)) {
          data += 7;
          switch (sz - 7) {
          case 4:
            if (!j::memcmp(data, "blue", 4)) {
              return styles::bright_blue;
            }
            if (!j::memcmp(data, "cyan", 4)) {
              return styles::bright_cyan;
            }
            break;
          case 5:
            if (!j::memcmp(data, "green", 5)) {
              return styles::bright_green;
            }
            break;
          case 6:
            if (!j::memcmp(data, "yellow", 6)) {
              return styles::bright_yellow;
            }
            break;
          case 7:
            if (!j::memcmp(data, "magenta", 7)) {
              return styles::bright_magenta;
            }
            break;
          }
        }
      }

      }
      return m_styles.at(str);
    }
  private:
    string_map<style> m_styles;
  };

  J_A(NODESTROY) extern constinit util::singleton<style_registry> g_style_registry;
}

extern template struct j::util::singleton<j::strings::styling::style_registry>;
