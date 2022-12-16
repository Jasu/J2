#include "meta/parsing.hpp"

namespace j::meta::inline rules {
  struct test_case;
  struct term_matcher;
  J_A(RNN,NODISC) test_case * parse_test_case(base_parser_state & state);
  J_A(RNN,NODISC) matcher_base * parse_matcher(base_parser_state & state);
  J_A(RNN,NODISC) term_matcher * parse_term_matcher(base_parser_state & state);
}
