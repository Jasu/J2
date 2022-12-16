#include "streams/source.hpp"
#include "streams/sink.hpp"
#include "services/interface_definition.hpp"

namespace j::streams {
  sink::~sink() {
  }

  u32_t sink::write(const char * J_NOT_NULL str) noexcept {
    return write(str, ::j::strlen(str));
  }

  u32_t sink::write(const strings::const_string_view & str) noexcept {
    return write(str.data(), str.size());
  }


  bool sink::is_tty() const {
    return false;
  }

  source::~source() {
  }

  namespace {
    J_A(ND, NODESTROY) const services::interface_definition<sink> sink_def("streams.sink", "Sink");
    J_A(ND, NODESTROY) const services::interface_definition<source> source_def("streams.source", "Source");
  }
}
