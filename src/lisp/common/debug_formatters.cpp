#include "lisp/common/debug_formatters.hpp"

#include "strings/formatters/debug_enum_formatter.hpp"
#include "strings/styling/default_styles.hpp"

namespace j::lisp::inline common {
  namespace {
    namespace s = strings;
    namespace f = s::formatters;

    static const f::debug_enum_entry mem_width_entries[] = {
      [(u8_t)mem_width::none]  = { "None", s::styles::bright_red },
      [(u8_t)mem_width::byte]  = { "Byte", s::styles::bright_magenta },
      [(u8_t)mem_width::word]  = { "Word", s::styles::bright_magenta },
      [(u8_t)mem_width::dword] = { "Dword", s::styles::bright_magenta },
      [(u8_t)mem_width::qword] = { "Qword", s::styles::white },
    };

    static const f::debug_enum_entry mem_scale_entries[] = {
      [(u8_t)mem_scale::none]  = { "none", s::styles::bright_red },
      [(u8_t)mem_scale::byte]  = { "bytes", s::styles::bright_magenta },
      [(u8_t)mem_scale::word]  = { "words", s::styles::bright_magenta },
      [(u8_t)mem_scale::dword] = { "dwords", s::styles::bright_magenta },
      [(u8_t)mem_scale::qword] = { "qwords", s::styles::white },
      [(u8_t)mem_scale::width] = { "width", s::styles::bold },
    };

    static const f::debug_enum_entry truthiness_entries[] = {
      [(u8_t)truthiness::none]          = { "-", s::styles::error },
      [(u8_t)truthiness::unknown]       = { "?", s::styles::bright_yellow },
      [(u8_t)truthiness::always_falsy]  = { "F", s::styles::bright_red },
      [(u8_t)truthiness::always_truthy] = { "T", s::styles::bright_green },
    };

    J_A(ND, NODESTROY) const s::formatters::debug_enum_formatter<mem_width> mem_width_formatter(mem_width_entries);
    J_A(ND, NODESTROY) const s::formatters::debug_enum_formatter<mem_scale> mem_scale_formatter(mem_scale_entries);
    J_A(ND, NODESTROY) const s::formatters::debug_enum_formatter<truthiness> truthiness_formatter(truthiness_entries);
  }
  const strings::formatter_known_length<mem_width> & g_mem_width_formatter = mem_width_formatter;
  const strings::formatter_known_length<mem_scale> & g_mem_scale_formatter = mem_scale_formatter;
  const strings::formatter_known_length<truthiness> & g_truthiness_formatter = truthiness_formatter;
}
