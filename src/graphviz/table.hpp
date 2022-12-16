#pragma once

#include "strings/string.hpp"
#include "containers/vector.hpp"
#include "containers/trivial_array_fwd.hpp"
#include "graphviz/common.hpp"

namespace j::graphviz {
  enum align : u8_t {
    align_none = 0,
    align_right,
    align_left,
    align_center,
  };
  enum valign : u8_t {
    valign_none = 0,
    valign_top,
    valign_bottom,
    valign_middle,
  };

  struct table_style final {
    color bg_color{};
    color fg_color{};
    float font_size = 0;
    align align = align_none;
    valign valign = valign_none;
    bool bold:1 = false;
    bool italic:1 = false;
    i8_t padding = -1;
  };

  struct table_column final {
    strings::string value{};
    table_style style{};
    strings::string port{};
    strings::string href{};
    strings::string tooltip{};
    strings::string id{};
  };

  struct table_row final {
    J_BOILERPLATE(table_row, CTOR_NE_ND, MOVE_NE_ND, COPY_DEL)

    table_row(u32_t sz, const table_style & style);
    table_row(const table_style & style, table_column * J_NOT_NULL begin, u32_t sz);
    table_row(const table_style & style, strings::string * J_NOT_NULL begin, u32_t sz);

    trivial_array<table_column> cols;
    table_style style;
    bool is_flex = false;
    u32_t size() const noexcept {
      return cols.size();
    }
    const table_column & operator[] (u32_t i) const noexcept {
      return cols[i];
    }
    table_column & operator[] (u32_t i) noexcept {
      return cols[i];
    }
  };

  struct table final {
    J_A(AI,ND) inline table() noexcept = default;
    explicit table(const table_style & st) noexcept;
    void set_style(const table_style & st) noexcept;
    noncopyable_vector<table_row> rows{};
    table_style style{};
    strings::string port{};

    [[nodiscard]] table_row & push_row(u32_t width);
    [[nodiscard]] table_row & push_row(u32_t width, const table_style & style);
    table_row & push_row(table_column * J_NOT_NULL begin, u32_t sz);
    table_row & push_row(const table_style & style, strings::string * J_NOT_NULL begin, u32_t sz);

    template<u32_t N>
    J_A(AI,ND) inline table_row & push_row(table_column (&& begin)[N]) {
      return push_row(begin, N);
    }
    template<u32_t N>
    J_A(AI,ND) inline table_row & push_row(const table_style & style, strings::string (&& begin)[N]) {
      return push_row(style, begin, N);
    }

    template<typename... Args>
    J_A(AI,ND,NODISC) inline table_row & push_flex_row(u32_t width, Args && ... args) {
      table_row & row = push_row(width, static_cast<Args &&>(args)...);
      row.is_flex = true;
      return row;
    }

    J_A(AI,ND) inline table_row & push_flex_row(table_column * J_NOT_NULL begin, u32_t sz) {
      table_row & row = push_row(begin, sz);
      row.is_flex = true;
      return row;
    }

    template<u32_t N>
    J_A(AI,ND) inline table_row & push_flex_row(table_column (&& begin)[N]) {
      table_row & row = push_row(begin, N);
      row.is_flex = true;
      return row;
    }

    void clear() noexcept;
    [[nodiscard]] strings::string render() const noexcept;
  };
}

J_DECLARE_EXTERN_TRIVIAL_ARRAY(j::graphviz::table_column);
