#pragma once

#include "hzd/utility.hpp"

namespace j::strings::regex {
  enum regex_newline : u8_t {
    nl_lf        = 0b0000001,
    nl_vt        = 0b0000010,
    nl_ff        = 0b0000100,
    nl_cr        = 0b0001000,
    nl_ls        = 0b0010000,
    nl_ps        = 0b0100000,
    nl_crlf      = 0b1000000,
    nl_ascii     = 0b1001001,
    nl_all_ascii = 0b1001111,
    nl_all       = 0b1111111,
  };

  enum regex_option_flag : u8_t {
    opt_none = 0b0000,
    opt_utf8 = 0b0001,
  };

  struct regex_options {
    u8_t flags = opt_none;
    regex_newline newline;
    J_A(AI,ND) inline explicit regex_options(u8_t flags, regex_newline nl = nl_ascii) noexcept
      : flags(flags),
        newline(nl)
    { }

    J_A(AI,NODISC) inline bool is_utf8() const noexcept { return flags & opt_utf8; }
  };
}
