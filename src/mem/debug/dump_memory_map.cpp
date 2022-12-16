#include "mem/debug/dump_memory_map.hpp"
#include "mem/memmap.hpp"

#include "services/services.hpp"
#include "strings/formatting/formatted_sink.hpp"

namespace j::mem::debug {
  void dump_memory_map(strings::formatted_sink & sink) {
    auto maps = read_mem_maps();
    for (auto & map : maps) {
      sink.write("{}\n", map);
    }
  }

  void dump_memory_map() {
    auto sink = services::service_container->get<strings::formatted_sink>(
      services::service_reference("stderr")
    );
    J_ASSERT_NOT_NULL(sink);
    dump_memory_map(*sink);
  }
}
