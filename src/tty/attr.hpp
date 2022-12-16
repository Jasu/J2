#pragma once

#include "mem/shared_ptr_fwd.hpp"

extern "C" {
  struct termpaint_attr_;
}

namespace j::tty {
  using attr = j::mem::shared_ptr<termpaint_attr_>;
}
