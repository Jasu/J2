#pragma once

#include "strings/styling/style.hpp"

namespace j::mem::debug::styles {
  extern const strings::style flag_readable;
  extern const strings::style flag_unreadable;
  extern const strings::style flag_writable;
  extern const strings::style flag_unwritable;
  extern const strings::style flag_wx;
  extern const strings::style flag_executable;
  extern const strings::style flag_unexecutable;
  extern const strings::style flag_private;
  extern const strings::style flag_shared;

  extern const strings::style reg_anonymous;
  extern const strings::style reg_self;

  extern const strings::style reg_system_lib;
  extern const strings::style reg_system_data;
  extern const strings::style reg_system_cache;

  extern const strings::style reg_heap;
  extern const strings::style reg_stack;
  extern const strings::style reg_vvar;
  extern const strings::style reg_vdso;
  extern const strings::style reg_vsyscall;

  extern const strings::style reg_memfd;
  extern const strings::style reg_anon_inode;

  extern const strings::style reg_other_file;
}
