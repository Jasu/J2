#pragma once

#include "exceptions/assert_lite.hpp"
#include "lisp/sources/source_location.hpp"

namespace j::lisp::mem {
  namespace detail {
    enum class source_location_tag_t : u8_t { v };
  }
  constexpr inline detail::source_location_tag_t source_location_tag{detail::source_location_tag_t::v};

  enum class debug_info_type : u8_t {
    source_location = 0U,
  };
}
