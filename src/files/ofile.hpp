#pragma once

#include "streams/fd_sink.hpp"
#include "files/open_flags.hpp"

namespace j::files {
  inline namespace paths {
    class path;
  }
  class ofile : public streams::fd_sink {
  public:
    constexpr ofile() noexcept = default;
    explicit ofile(const path & p, open_flags_t o = open_flags::truncate | open_flags::create);
    ofile(ofile &&) noexcept = default;
    ofile & operator=(ofile &&) noexcept = default;


    void open(const path & p, open_flags_t o = open_flags::truncate | open_flags::create);
    void write_bytes(const char * from, u32_t num_bytes);
    void close();

    virtual ~ofile();
  };
}
