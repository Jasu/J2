#pragma once

#include "hzd/utility.hpp"

namespace j::inline containers {
  enum copy_tag : u8_t { copy };
  enum move_tag : u8_t { move };
  enum uninitialized_tag : u8_t { uninitialized };
  enum in_place_tag :u8_t { in_place };
}
