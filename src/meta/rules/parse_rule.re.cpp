#include "exceptions/assert.hpp"
#include "meta/parsing.hpp"
#include "meta/rules/pattern_tree.hpp"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunreachable-code"
#pragma clang diagnostic ignored "-Wunused"
#pragma clang diagnostic ignored "-Wunused-parameter"
#pragma clang diagnostic ignored "-Wunused-label"
#pragma clang diagnostic ignored "-Wunused-variable"

/*!max:re2c*/
#ifndef YYMAXFILL
#define YYMAXFILL 5
#endif

/*!rules:re2c:common
  re2c:api:style             = free-form;
  re2c:define:YYCTYPE        = "char";
  re2c:define:YYCURSOR       = "state.cursor";
  re2c:define:YYLIMIT        = "state.limit";
  re2c:define:YYMARKER       = "marker";
  re2c:define:YYFILL         = "J_UNREACHABLE();";

  // Allow rules to have @var_name to store the position in the variable.
  re2c:flags:tags = 1;

  NL         = [\n];
  COMMENT    = "#" [^\n]*;
  WS         = [ ]+;
  NAME       = [A-Z][A-Za-z0-9]*;
  ID         = [a-z][a-z0-9_-]*;
*/

namespace j::meta::inline rules {
  namespace {
    using pat_term_exits = vector<pat_terms_t*>;

    struct J_TYPE_HIDDEN pat_term_chunk final {
      pat_terms_t entries;
      pat_term_exits exits;

      pat_term_chunk() noexcept = default;

      void set_exits(pat_terms_t * J_NOT_NULL exits) {
        if (!*exits) {
          this->exits.push_back(exits);
        } else {
          for (auto & t : *exits) {
            set_exits(&t->next);
          }
        }
      }

      pat_term_chunk clone() const noexcept {
        pat_term_chunk result;
        result.entries.reserve(entries.size());
        for (auto & t : entries) {
          result.set_exits(&result.entries.push_back(t->clone())->next);
        }
        return result;
      }

      J_A(AI,NODISC,ND,HIDDEN) inline explicit operator bool() const noexcept {
        return (bool)entries;
      }
      J_A(AI,NODISC,ND,HIDDEN) inline bool operator!() const noexcept {
        return !(bool)entries;
      }

      explicit pat_term_chunk(pat_terms_t * J_NOT_NULL out) noexcept
        : exits(1)
      {
        exits.push_back(out);
      }

      explicit pat_term_chunk(pat_terms_t && terms) noexcept
        : entries(static_cast<pat_terms_t &&>(terms)),
          exits(entries.size())
      {
        for (auto & in : entries) {
          J_ASSERT(in && !in->next);
          exits.push_back(&in->next);
        }
      }

      pat_term_chunk & set_next(pat_term_chunk && rhs) noexcept {
        if (rhs) {
          if (!entries) {
            entries = static_cast<pat_terms_t &&>(rhs.entries);
          }
          connect_entries(rhs.entries);
          exits = static_cast<pat_term_exits &&>(rhs.exits);
        }
        return *this;
      }

      pat_term_chunk & set_next(const pat_term_chunk & rhs) noexcept {
        if (rhs) {
          if (!entries) {
            entries = rhs.entries;
          }
          connect_entries(rhs.entries);
          exits = rhs.exits;
        }
        return *this;
      }

      pat_term_chunk & set_next(const pat_term_p & rhs) noexcept {
        if (!entries) {
          entries.push_back(rhs);
        }
        for (auto exit : exits) {
          J_ASSERT(exit && !*exit);
          exit->push_back(rhs);
        }
        exits.resize(1);
        exits[0] = &rhs->next;
        return *this;
      }

      pat_term_chunk & add_sibling(pat_term_chunk && rhs) noexcept {
        exits.reserve(exits.size() + rhs.exits.size());
        entries.reserve(entries.size() + rhs.entries.size());
        for (auto & in : rhs.entries) {
          entries.push_back(static_cast<pat_term_p &&>(in));
        }
        for (auto & in : rhs.exits) {
          exits.push_back(in);
        }
        return *this;
      }

    private:
      void connect_entries(const pat_terms_t & entries) {
        for (auto & exit : exits) {
          J_ASSERT(exit && !*exit);
          *exit = entries;
        }
      }
    };

    namespace s = j::strings;
    J_A(NODISC) pat_term_p parse_pat_term_base(base_parser_state & state, bool & at_end) {
      const char *begin;
      pat_term::pat_term_type type = pat_term::type_none;
      i32_t capture = -1;
      /*!stags:re2c:term_base format = "const char *@@ = nullptr;\n"; */
    loop:
    /*!local:re2c:term_base
      !use:common;

      WS | COMMENT { goto loop; }
      NL { state.newline(); goto loop; }

      "_" { type = pat_term::type_any; goto make_pat; }
      "^" { type = pat_term::type_anchor; goto make_pat; }
      "$" [1-9] {
        type = pat_term::type_any;
        capture = state.cursor[-1] - '0';
        goto make_pat;
      }
      "$" {
        at_end = true;
        type = pat_term::type_end;
        goto make_pat;
      }
      @begin ID { return j::mem::make_shared<pat_term>(pat_term::type_term, state.term_at(s::const_string_view(begin, state.cursor))); }
      @begin NAME { return j::mem::make_shared<pat_term>(state.nt_at(s::const_string_view(begin, state.cursor))); }
      [\x00] { state.fail("Unexpected eof", "in rule pattern"); }
      * { state.fail("Unexpected token", "in rule pattern"); }
      */
      J_UNREACHABLE();
    make_pat:
      return j::mem::make_shared<pat_term>(type, capture);
    }

    void parse_nt_args(base_parser_state & state, pat_terms_t & to) {
      const char *marker;
      i32_t index = 0;
    loop:
      /*!local:re2c:nt_args
        !use:common;
        WS | COMMENT { goto loop; }
        NL { state.newline(); goto loop; }
        "_" { ++index; goto loop; }
        "$" [1-9] {
          for (auto & l : to) {
            l->nt_captures[index] = state.cursor[-1] - '0';
          }
          ++index;
          goto loop;
        }
        ")" { return; }
        [\x00] { state.fail("Unexpected EOF", "in non-terminal captures"); }
        * { state.fail("Unexpected token", "in non-terminal captures"); }
        */
      J_UNREACHABLE();
    }

    void set_capture(pat_terms_t & to, i32_t capture_index) noexcept {
      J_ASSERT_NOT_NULL(to);
      if (capture_index >= 0) {
        for (pat_term_p & term : to) {
          J_ASSERT(term->capture_index < 0);
          term->capture_index = capture_index;
        }
      }
    }

    pat_term_chunk parse_pat_term(base_parser_state & state, bool & at_end);
    pat_term_chunk parse_pat_term_or_operator(base_parser_state & state, bool & at_end) {
      pat_term_chunk lhs = parse_pat_term(state, at_end);
	     if (skip_ws(state) == '<' && state.cursor[0] == '-' && state.cursor[1] == '>') {
        state.cursor += 2;
        skip_ws(state);
        --state.cursor;
        pat_term_chunk rhs = parse_pat_term(state, at_end);
        pat_term_chunk rhs2 = rhs.clone();
        rhs2.set_next(lhs.clone());
        lhs.set_next(rhs);
        lhs.add_sibling(static_cast<pat_term_chunk &&>(rhs2));
        J_ASSERT(!at_end);
      } else {
        --state.cursor;
      }
      return lhs;
    }

    pat_term_chunk parse_pat_terms(base_parser_state & state, bool & at_end, char end_char) {
      pat_term_chunk result;
      while (skip_ws(state) != end_char) {
        --state.cursor;
        result.set_next(parse_pat_term_or_operator(state, at_end));
      }
      return result;
    }

    pat_term_chunk parse_pat_term(base_parser_state & state, bool & at_end) {
      pat_terms_t terms(1);
      terms.push_back(parse_pat_term_base(state, at_end));
      bool has_args = false;
      i32_t capture_index = -1;
      const char *marker;
    loop:
      /*!local:re2c:term_tail
        !use:common;
        "|" {
          terms.push_back(parse_pat_term_base(state, at_end));
          goto loop;
        }
        ":$" [1-9] {
          state.check(capture_index == -1, "Multiple captures", "in pat term");
          capture_index = state.cursor[-1] - '0';
          goto loop;
        }
        "(" { has_args = true; goto out; }
        * { --state.cursor; goto out; }
        [\x00] { state.fail("Unexpected EOF", "in pat term"); }
        */
      J_UNREACHABLE();
    out:
      pat_term_chunk result(static_cast<pat_terms_t &&>(terms));
      J_ASSERT_NOT_NULL(result, result.entries, result.exits);
      if (has_args) {
        if (result.entries[0]->type == pat_term::type_term) {
          for (auto & t : result.entries) {
            state.check(t->type == pat_term::type_term, "Mixed alternates with shared args", "in pat term");
            t->type = pat_term::type_ctor;
          }
          result.set_next(parse_pat_terms(state, at_end, ')'));
          J_ASSERT_NOT_NULL(result, result.entries, result.exits);
          if (!at_end) {
            result.set_next(j::mem::make_shared<pat_term>(pat_term::type_exit, capture_index));
            J_ASSERT_NOT_NULL(result, result.entries, result.exits);
            capture_index = -1;
          }
        } else {
          parse_nt_args(state, result.entries);
        }
      }
      set_capture(result.entries, capture_index);
      return result;
    }

  }

  void parse_pat_terms(base_parser_state & state, pat_terms_t * J_NOT_NULL out) {
    bool at_end = false;
    pat_term_chunk(out).set_next(parse_pat_terms(state, at_end, '-'));
    J_ASSERT_NOT_NULL(*out);
    --state.cursor;
  }

  J_A(NODISC) pat_terms_t parse_pat_terms(base_parser_state & state) {
    pat_terms_t result;
    parse_pat_terms(state, &result);
    --state.cursor;
    J_ASSERT(result);
    return result;
  }
}

#pragma clang diagnostic pop
