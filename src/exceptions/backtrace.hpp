#pragma once

#include "backtrace/stored_backtrace.hpp"
#include "exceptions/exceptions.hpp"

namespace j::exceptions {
  extern const tags::tag_definition<::j::backtrace::stored_backtrace> backtrace;
}
