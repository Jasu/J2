#pragma once

namespace j::files::inline paths {
  class path;
}
namespace j::rendering::vulkan::state {
  class state_tracker;
  void graphviz_dump(const files::path & path, state_tracker & tracker);
}
