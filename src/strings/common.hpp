#pragma once

namespace j::strings {
  namespace detail {
    enum class null_terminate_tag_t { v };
    enum class maybe_null_terminate_tag_t { v };
  }
  inline constexpr detail::null_terminate_tag_t null_terminate
    = detail::null_terminate_tag_t::v;
  inline constexpr detail::maybe_null_terminate_tag_t maybe_null_terminate
    = detail::maybe_null_terminate_tag_t::v;
}
