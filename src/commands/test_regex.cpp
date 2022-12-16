#include "cli/cli.hpp"
#include "services/service_instance.hpp"
#include "strings/formatting/formatted_sink.hpp"
#include "strings/regex/graphviz.hpp"
#include "strings/regex/nfa_match.hpp"

namespace j::commands {
  namespace {
    namespace s = j::strings;
    namespace r = j::strings::regex;
    void test_regex(mem::shared_ptr<s::formatted_sink> out, s::const_string_view rx, s::const_string_view sub) {
      auto node = s::regex::read(rx);
      r::dump_full(node);
      r::nfa * nfa = s::regex::to_nfa(node);
      r::graphviz_dump("regex.dot", rx, nfa);
      r::regex_matcher<const char*> matcher(nfa, sub.begin(), sub.end());
      auto match = matcher.match();
      if (!match) {
        out->write("No match for {}.\n", sub);
      } else {
        out->write("Match for {}:\n", sub);
        out->write("  \"{}\"\n", s::const_string_view(match.begin, match.end));
      }
      ::delete nfa;
    }

    namespace c = cli;
    namespace svc = services;
    J_A(ND,NODESTROY) const svc::service_instance<c::command> cmd(
      "commands.test_regex",
      "Test regular expressions",

      c::command_name = "test-regex",
      c::callback     = svc::call(&test_regex, svc::service("stdout"),
                                  c::argument("regex", c::default_value = ""),
                                  c::argument("subject", c::default_value = "")));
  }
}
