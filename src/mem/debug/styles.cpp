#include "mem/debug/styles.hpp"

#include "colors/default_colors.hpp"

namespace j::mem::debug::styles {
  namespace s = j::strings;
  using color = colors::color;
  namespace c = colors::colors;
  const color gray{colors::rgb8{128,128,128}, colors::ansi_color::bright_black};

  const s::style flag_readable{c::white, c::green};
  const s::style flag_unreadable{c::bright_yellow, gray};
  const s::style flag_writable{c::black, c::bright_yellow};
  const s::style flag_unwritable{c::bright_yellow, gray};
  const s::style flag_wx{c::bright_yellow, c::bright_red, s::bold};
  const s::style flag_executable{c::white, c::bright_magenta, s::bold};
  const s::style flag_unexecutable{c::white, gray};
  const s::style flag_private{c::black, gray};
  const s::style flag_shared{c::white, c::bright_cyan, s::bold};

  const s::style reg_anonymous{c::bright_green};
  const s::style reg_self{c::bright_magenta};
  const s::style reg_system_lib{c::light_gray};
  const s::style reg_system_data{
    color{colors::rgb8{64, 100, 240}, colors::ansi_color::blue},
    s::bold
  };
  const s::style reg_system_cache{
    color{colors::rgb8{188, 188, 124}, colors::ansi_color::yellow},
    s::bold
  };
  const s::style reg_stack{
    color{colors::rgb8{255U,200U,20U}, colors::ansi_color::yellow},
    s::bold,
  };

  const s::style reg_heap{
    color{colors::rgb8{25U,200U,255U}, colors::ansi_color::bright_cyan},
    s::bold,
  };
  const s::style reg_vvar{c::bright_blue};
  const s::style reg_vdso{c::bright_cyan};
  const s::style reg_vsyscall{c::blue, s::bold};

  const s::style reg_memfd{
    color{colors::rgb8{91U,224U,255U}, colors::ansi_color::bright_cyan},
    s::bold,
  };
  const s::style reg_anon_inode{
    color{colors::rgb8{255U,165U,85U}, colors::ansi_color::yellow},
    s::bold,
  };

  const s::style reg_other_file{c::bright_yellow};
}
