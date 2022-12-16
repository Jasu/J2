#pragma once

#include "strings/unicode/rope/rope_utf8_value.hpp"

namespace j::strings::inline unicode::inline rope {
  using rope_utf8_tree = t::rope_tree<rope_utf8_description>;
  using rope_utf8_chunk_iterator = rope_utf8_tree::iterator;
  using const_rope_utf8_chunk_iterator = rope_utf8_tree::const_iterator;
}
