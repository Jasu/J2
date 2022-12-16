#include "logging/severity.hpp"
#include "strings/formatters/enum_formatter.hpp"
#include "strings/styling/style.hpp"
#include "strings/styling/style_registry.hpp"

namespace j::logging {
  namespace {
    namespace s = strings;
    J_A(ND, NODESTROY) const s::formatters::enum_formatter<severity> severity_formatter{{
      {severity::emergency, "  Emerg ", s::g_style_registry->at("emergency")},
      {severity::alert,     "  Alert ", s::g_style_registry->at("alert")},
      {severity::critical,  "  Crit  ", s::g_style_registry->at("critical")},
      {severity::error,     "  Error ", s::g_style_registry->at("error")},
      {severity::warning,   "  Warn  ", s::g_style_registry->at("warning")},
      {severity::notice,    " Notice ", s::g_style_registry->at("notice")},
      {severity::info,      "  Info  ", s::g_style_registry->at("info")},
      {severity::debug,     "  Debug ", s::g_style_registry->at("debug")},
    }};
  }

}
