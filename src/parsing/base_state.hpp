#pragma once

#include "strings/string_view.hpp"

namespace j::parsing {
  struct base_state {
    J_BOILERPLATE(base_state, COPY_DEL)

    J_A(AI,ND) inline base_state(const char * J_NOT_NULL cursor, const char * J_NOT_NULL limit, strings::const_string_view buffer_name)
      : cursor(cursor),
        limit(limit),
        line_begin(cursor),
        buffer_name(buffer_name)
    {
    }

    const char * cursor = nullptr;
    const char * marker = nullptr;
    const char * limit = nullptr;

    const char * line_begin = nullptr;
    u32_t line_number = 1U;
    strings::const_string_view buffer_name{};

    J_A(AI,ND) inline void newline() noexcept {
      line_begin = cursor;
      ++line_number;
    }

    J_A(NORET,COLD) void fail(strings::const_string_view msg) const;

    J_A(NORET,COLD) void unexpected(strings::const_string_view ctx) const;
    J_A(NORET,COLD) void unexpected(strings::const_string_view ctx, char c) const;

    J_A(AI) inline void check(bool pred, strings::const_string_view msg) const {
      if (!pred) {
        fail(msg);
      }
    }

    i64_t parse_int();
    i64_t parse_int(const char * J_NOT_NULL begin) const;

    i64_t parse_dec();
    i64_t parse_dec(const char * J_NOT_NULL begin) const;

    i64_t parse_bin();
    i64_t parse_bin(const char * J_NOT_NULL begin) const;

    i64_t parse_hex();
    i64_t parse_hex(const char * J_NOT_NULL begin) const;
  };
}
