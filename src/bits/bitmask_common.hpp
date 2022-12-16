#pragma once

namespace j::bits {
  namespace detail {
    struct for_overwrite_tag { };
  }
  inline constexpr const detail::for_overwrite_tag for_overwrite_tag;
}
