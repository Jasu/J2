#pragma once

namespace j::strings::inline formatting {
  class formatted_sink;
}

namespace j::mem::debug {
  void dump_memory_map(strings::formatted_sink & sink);
  void dump_memory_map();
}

#define J_DUMP_MEMORY_MAP j::mem::debug::dump_memory_map
