#pragma once

#include "containers/pair.hpp"
#include "containers/static_array.hpp"
#include "lisp/common/metadata.hpp"

namespace j::lisp::inline common {
  struct value_metadata_init final {
    id id{};
    i8_t default_from = -2;
    operand_name_format name_format = operand_name_format::default_format;
    format_hint format = format_hint::none;
    strings::const_string_view name;
  };

  struct metadata_init_loc;
  struct metadata_init_comment;

  struct metadata_init final {
    sources::source_location loc;
    strings::const_string_view comment;
    value_metadata_init result;

    i8_t default_value_offset = -2;
    value_metadata_init args[4] = {};
    const metadata_view * src = nullptr;


    [[nodiscard]] u32_t size() const noexcept;
    void write(char * J_NOT_NULL to) const noexcept;
  };

  struct value_mapping final {
    i8_t to = -2, from = -2;
    inline void apply(metadata_init & md, const metadata_init & src) const noexcept {
      if (to == -3) {
        md.default_value_offset = md.default_value_offset > 0 ? md.default_value_offset + from : from;
      } else if (to == -1) {
        md.result = from == -1 ? src.result : src.args[to];
      } else if (to != -2) {
        md.args[to] = from == -1 ? src.result : src.args[to];
      }
    }
  };

  struct metadata_mapping final {
    value_mapping values[4]{};
    inline metadata_init apply(const metadata_init & md) const noexcept {
      metadata_init result;
      for (auto & v : values) {
        v.apply(result, md);
      }
      return result;
    }
  };

  constexpr inline value_mapping result_from_result = {-1,-1};

  consteval inline value_mapping result_from(i8_t index) noexcept {
    return {-1, index};
  }

  consteval value_mapping arg_from_result(i8_t i) noexcept {
    return {i, -1};
  }

  consteval inline value_mapping arg_from(i8_t i, i8_t index) noexcept {
    return {i, index};
  }
  consteval inline value_mapping args_from(i8_t index) noexcept {
    return {-3, index};
  }
  template<typename... Args>
  consteval metadata_mapping metadata_map(Args ... args) noexcept {
    return metadata_mapping{{args...}};
  }




  J_A(AI,ND,NODISC) inline metadata_init copy_metadata(const metadata_view & from) noexcept {
    return metadata_init{
    .result{.default_from = -1},
    .default_value_offset = 0,
    .args{{.default_from = 0}, {.default_from = 1}, {.default_from = 2}, {.default_from = 3}},
    .src = &from,
    };
  }

  namespace mdi {
    struct copy final {
      const metadata_view * md = nullptr;
      inline copy(const metadata_view * J_NOT_NULL md)
        : md(md) {
      }
      inline copy(const metadata_view & md)
        : md(&md) {
      }
    };

    struct loc final {
      sources::source_location l;
      constexpr inline loc(sources::source_location l) noexcept
        : l(l) {
      }
    };

    struct comment final {
      strings::const_string_view c;
      constexpr inline comment(strings::const_string_view c) noexcept
        : c(c) {
      }
    };

    struct result final {
      strings::const_string_view s;
      id n{};
      constexpr inline result(strings::const_string_view s) noexcept
        : s(s) {
      }
      inline result(id n) noexcept
        : n(n) {
      }
    };

    struct op final {
      u8_t i;
      strings::const_string_view s;
      id n{};
      constexpr inline op(u8_t i, strings::const_string_view s) noexcept
        : i(i),
          s(s) {
      }
      inline op(u8_t i, id n) noexcept
        : i(i),
          n(n) {
      }
    };

    J_A(AI,ND) inline void apply(metadata_init & md, const loc & l) noexcept {
      md.loc = l.l;
    }

    J_A(AI,ND) inline void apply(metadata_init & md, const comment & c) noexcept {
      md.comment = c.c;
    }

    J_A(AI,ND) inline void apply(metadata_init & md, const result & r) noexcept {
      if (r.s) {
        md.result.name = r.s;
      }
      if (r.n) {
        md.result.id = r.n;
      }
    }
    J_A(AI,ND) inline void apply(metadata_init & md, const op & o) noexcept {
      if (o.s) {
        md.args[o.i].name = o.s;
      }
      if (o.n) {
        md.args[o.i].id = o.n;
      }
    }

    J_A(AI,ND) inline void apply(metadata_init & md, const copy & r) noexcept {
      if (md.result.default_from == -2) {
        md.result.default_from = -1;
      }
      if (md.default_value_offset == -2) {
        md.default_value_offset = 0;
      }
      for (u8_t i = 0; i < J_ARRAY_SIZE(md.args); ++i) {
        if (md.args[i].default_from == -2) {
          md.args[i].default_from = i;
        }
      }
      md.src = r.md;
    }

    J_A(AI,ND) inline void apply(metadata_init & md, const metadata_view & v) noexcept {
      md.src = &v;
    }
  }

  template<typename... Args>
  J_A(AI,ND,NODISC) metadata_init with_metadata(Args && ... args) {
    metadata_init result;
    (mdi::apply(result, args), ...);
    return result;
  }
}
