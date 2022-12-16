#pragma once

#include "strings/regex/nfa.hpp"
#include "files/paths/path.hpp"

namespace j::strings::regex {
  void graphviz_dump(const files::path & path, const_string_view regex, const nfa * J_AA(NN) nfa);
}
