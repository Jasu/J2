#pragma once

namespace j::streams {
  class sink;
}

namespace j::mem {
  template<typename T>
  class shared_ptr;
}

namespace j::graphviz {
  struct graph;
  void serialize(const graph & graph, mem::shared_ptr<streams::sink> stream);
}
