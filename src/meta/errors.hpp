#pragma once

#include "exceptions/exceptions.hpp"
#include "hzd/utility.hpp"

namespace j::files::inline paths {
  class path;
}
namespace j::meta {
  struct source_location final {
    i32_t line = -1;
    i32_t column = -1;
    const files::path * cur_file = nullptr;
    J_A(AI,NODISC,ND) inline explicit operator bool() const noexcept {
      return line >= 0 || column >= 0 || cur_file;
    }
    J_A(AI,NODISC,ND) inline bool operator!() const noexcept {
      return !(bool)*this;
    }
  };

  enum err_severity : u8_t {
    err_notice,
    err_warning,
    err_error,
  };

  struct error final {
    J_A(AI,ND) inline error() noexcept = default;
    error(const char * J_NOT_NULL msg, source_location loc, err_severity sev = err_error) noexcept;
    error(strings::string && msg, source_location loc, err_severity sev = err_error) noexcept;

    strings::string msg;
    source_location loc;
    err_severity sev;
  };

  extern const tags::tag_definition<source_location> source_loc;
}
