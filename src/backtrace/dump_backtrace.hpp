#pragma once

namespace j::strings::inline formatting {
  class formatted_sink;
}

namespace j::backtrace {
  void dump_backtrace(strings::formatted_sink & sink, int skip = 0);
  void dump_backtrace(int skip = 0);
}
#define J_BACKTRACE ::j::backtrace::dump_backtrace
