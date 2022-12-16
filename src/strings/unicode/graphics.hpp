#pragma once

#include "strings/string_view.hpp"

namespace j::strings::inline unicode {
  constexpr inline const char * const g_cstr_arrow_left = "←";
  constexpr inline const_string_view g_arrow_left{g_cstr_arrow_left};
  constexpr inline const char * const g_cstr_arrow_right = "→";
  constexpr inline const_string_view g_arrow_right{g_cstr_arrow_right};
  constexpr inline const char * const g_cstr_arrow_up = "↑";
  constexpr inline const_string_view g_arrow_up{g_cstr_arrow_up};
  constexpr inline const char * const g_cstr_arrow_down = "↓";
  constexpr inline const_string_view g_arrow_down{g_cstr_arrow_down};

  enum class box_type_weight : u8_t {
    light,
    heavy,
  };

  enum class box_type_base : u8_t {
    light,
    double_light,
    heavy,
  };

  enum class box_type : u8_t {
    none,
    light,
    light_round,
    dashed,
    light_triple_dash,
    light_quadruple_dash,
    double_light,
    heavy,
    heavy_dashed,
    heavy_triple_dash,
    heavy_quadruple_dash,
  };

  J_INLINE_GETTER constexpr box_type_base to_box_type_base(box_type t) {
    if (t < box_type::double_light) {
      return box_type_base::light;
    } else if (t == box_type::double_light) {
      return box_type_base::double_light;
    } else {
      return box_type_base::heavy;
    }
  }

  J_INLINE_GETTER constexpr box_type_weight to_box_type_weight(box_type t) {
    if (t < box_type::double_light) {
      return box_type_weight::light;
    } else {
      return box_type_weight::heavy;
    }
  }

  inline constexpr const char * vertical_lines[]   {"│", "│", "│", "╎", "┆", "┊", "║", "┃", "╏", "┇", "┋"};
  inline constexpr const char * horizontal_lines[] {"─", "─", "─", "╌", "┄", "┈", "═", "━", "╍", "┅", "┉"};
  inline constexpr const char * nw_corners[]       {"┌", "┌", "╭", "┌", "┌", "┌", "╔", "┏", "┏", "┏", "┏"};
  inline constexpr const char * sw_corners[]       {"└", "└", "╰", "└", "└", "└", "╚", "┗", "┗", "┗", "┗"};
  inline constexpr const char * ne_corners[]       {"┐", "┐", "╮", "┐", "┐", "┐", "╗", "┓", "┓", "┓", "┓"};
  inline constexpr const char * se_corners[]       {"┘", "┘", "╯", "┘", "┘", "┘", "╝", "┛", "┛", "┛", "┛"};


  constexpr inline const_string_view vertical_line(box_type from, box_type to) {
    if (J_UNLIKELY(from != to)) {
      if (from < box_type::heavy && to >= box_type::heavy) {
        return "╽";
      } else if (from >= box_type::heavy && to < box_type::heavy) {
        return "╿";
      }
    }
    return vertical_lines[static_cast<u8_t>(from)];
  }

  constexpr inline const_string_view horizontal_line(box_type from, box_type to) {
    if (J_UNLIKELY(from != to)) {
      if (from < box_type::heavy && to >= box_type::heavy) {
        return "╼";
      } else if (from >= box_type::heavy && to < box_type::heavy) {
        return "╾";
      }
    }
    return horizontal_lines[static_cast<u8_t>(from)];
  }


  constexpr inline const_string_view nw_corner(box_type from, box_type to) {
    if (J_UNLIKELY(from != to)) {
      if (from < box_type::heavy && to >= box_type::heavy) {
        return "┍";
      } else if (from >= box_type::heavy && to < box_type::heavy) {
        return "┎";
      } else if (from == box_type::double_light) {
        return "╓";
      } else if (to == box_type::double_light) {
        return "╒";
      } else if (to == box_type::light_round) {
        from = box_type::light_round;
      }
    }
    return nw_corners[(u8_t)from];
  }

  constexpr inline const_string_view ne_corner(box_type from, box_type to) {
    if (J_UNLIKELY(from != to)) {
      if (from < box_type::heavy && to >= box_type::heavy) {
        return "┒";
      } else if (from >= box_type::heavy && to < box_type::heavy) {
        return "┑";
      } else if (from == box_type::double_light) {
        return "╕";
      } else if (to == box_type::double_light) {
        return "╖";
      } else if (to == box_type::light_round) {
        from = box_type::light_round;
      }
    }
    return ne_corners[(u8_t)from];
  }

  constexpr inline const_string_view sw_corner(box_type from, box_type to) {
    if (J_UNLIKELY(from != to)) {
      if (from < box_type::heavy && to >= box_type::heavy) {
        return "┖";
      } else if (from >= box_type::heavy && to < box_type::heavy) {
        return "┕";
      } else if (from == box_type::double_light) {
        return "╘";
      } else if (to == box_type::double_light) {
        return "╙";
      } else if (to == box_type::light_round) {
        from = box_type::light_round;
      }
    }
    return sw_corners[(u8_t)from];
  }

  constexpr inline const_string_view se_corner(box_type from, box_type to) {
    if (J_UNLIKELY(from != to)) {
      if (from < box_type::heavy && to >= box_type::heavy) {
        return "┙";
      } else if (from >= box_type::heavy && to < box_type::heavy) {
        return "┚";
      } else if (from == box_type::double_light) {
        return "╜";
      } else if (to == box_type::double_light) {
        return "╛";
      } else if (to == box_type::light_round) {
        from = box_type::light_round;
      }
    }
    return se_corners[(u8_t)from];
  }

  constexpr inline const_string_view tee_right(box_type from, box_type to, box_type other) {
    auto w = to_box_type_weight(from), wo = to_box_type_weight(other);
    if (from == box_type::double_light) {
      return other == box_type::double_light ? "╠" : "╟";
    } else if (w == box_type_weight::heavy && to_box_type_weight(to) == box_type_weight::light) {
      return wo == box_type_weight::heavy ? "┡" : "┞";
    } else if (w == box_type_weight::light && to_box_type_weight(to) == box_type_weight::heavy) {
      return wo == box_type_weight::heavy ? "┢" : "┟";
    }
    if (w == box_type_weight::heavy) {
      return wo == box_type_weight::heavy ? "┣" : "┠";
    } else {
      if (other == box_type::double_light) {
        return "╞";
      }
      return wo == box_type_weight::heavy ? "┝" : "├";
    }
  }

  constexpr inline const_string_view tee_left(box_type from, box_type to, box_type other) {
    auto w = to_box_type_weight(from), wo = to_box_type_weight(other);
    if (from == box_type::double_light) {
      return other == box_type::double_light ? "╣" : "╢";
    } else if (w == box_type_weight::heavy && to_box_type_weight(to) == box_type_weight::light) {
      return wo == box_type_weight::heavy ? "┩" : "┦";
    } else if (w == box_type_weight::light && to_box_type_weight(to) == box_type_weight::heavy) {
      return wo == box_type_weight::heavy ? "┪" : "┧";
    }
    if (w == box_type_weight::heavy) {
      return wo == box_type_weight::heavy ? "┫" : "┨";
    } else {
      if (other == box_type::double_light) {
        return "╡";
      }
      return wo == box_type_weight::heavy ? "┥" : "┤";
    }
  }

  constexpr inline const_string_view tee_down(box_type from, box_type to, box_type other) {
    auto w = to_box_type_weight(from), wo = to_box_type_weight(other);
    if (from == box_type::double_light) {
      return other == box_type::double_light ? "╦" : "╤";
    } else if (w == box_type_weight::heavy && to_box_type_weight(to) == box_type_weight::light) {
      return wo == box_type_weight::heavy ? "┱" : "┭";
    } else if (w == box_type_weight::light && to_box_type_weight(to) == box_type_weight::heavy) {
      return wo == box_type_weight::heavy ? "┲" : "┮";
    }
    if (w == box_type_weight::heavy) {
      return wo == box_type_weight::heavy ? "┳" : "┯";
    } else {
      if (other == box_type::double_light) {
        return "╥";
      }
      return wo == box_type_weight::heavy ? "┰" : "┬";
    }
  }

  constexpr inline const_string_view tee_up(box_type from, box_type to, box_type other) {
    auto w = to_box_type_weight(from), wo = to_box_type_weight(other);
    if (from == box_type::double_light) {
      return other == box_type::double_light ? "╩" : "╧";
    } else if (w == box_type_weight::heavy && to_box_type_weight(to) == box_type_weight::light) {
      return wo == box_type_weight::heavy ? "┹" : "┵";
    } else if (w == box_type_weight::light && to_box_type_weight(to) == box_type_weight::heavy) {
      return wo == box_type_weight::heavy ? "┺" : "┶";
    }
    if (w == box_type_weight::heavy) {
      return wo == box_type_weight::heavy ? "┻" : "┷";
    } else {
      if (other == box_type::double_light) {
        return "╨";
      }
      return wo == box_type_weight::heavy ? "┸" : "┴";
    }
  }

  constexpr inline const char * heavy_crosses[] = {
    //      T B L R
    "┼", // - - - -
    "╀", // * - - -
    "╁", // - * - -
    "╂", // * * - -
    "┽", // - - * -
    "╃", // * - * -
    "╅", // - * * -
    "╉", // * * * -
    "┾", // - - - *
    "╄", // * - - *
    "╆", // - * - *
    "╊", // * * - *
    "┿", // - - * *
    "╇", // * - * *
    "╊", // - * * *
    "╋", // * * * *
  };

  constexpr inline const_string_view cross(box_type top, box_type bottom, box_type left,box_type right) {
    const auto top_b = to_box_type_base(top),   bottom_b = to_box_type_base(bottom),
               left_b = to_box_type_base(left), right_b = to_box_type_base(right);
    if (top_b == box_type_base::double_light && bottom_b == box_type_base::double_light) {
      if (left_b == box_type_base::double_light && right_b == box_type_base::double_light) {
        return "╬";
      }
      if (left_b == box_type_base::light && right_b == box_type_base::light) {
        return "╫";
      }
    } else if (left_b == box_type_base::double_light && right_b == box_type_base::double_light) {
      if (top_b == box_type_base::light && bottom_b == box_type_base::light) {
        return "╪";
      }
    }

    const u8_t index = (top_b    == box_type_base::light ? 0U : 1U)
                     + (bottom_b == box_type_base::light ? 0U : 2U)
                     + (left_b   == box_type_base::light ? 0U : 4U)
                     + (right_b  == box_type_base::light ? 0U : 8U);
    return heavy_crosses[index];
  }

  struct box_drawing_context final {
    box_type above = box_type::none, below = box_type::none, left = box_type::none, right = box_type::none;
  };

  constexpr inline const_string_view get_box_character(box_drawing_context c) {
    const u32_t num = (c.above != box_type::none) + (c.below != box_type::none)
                    + (c.left != box_type::none) + (c.right != box_type::none);
    if (num < 2) {
      return " ";
    }

    if (num == 4) {
      return cross(c.above, c.below, c.left, c.right);
    }

    if (num == 3) {
      if (c.left == box_type::none) {
        return tee_right(c.above, c.below, c.right);
      } else if (c.right == box_type::none) {
        return tee_left(c.above, c.below, c.left);
      } else if (c.above == box_type::none) {
        return tee_down(c.left, c.right, c.below);
      }
      return tee_up(c.left, c.right, c.above);
    }

    // Horizontal lines
    if (c.left == box_type::none && c.right == box_type::none) {
      return vertical_line(c.above, c.below);
    } else if (c.above == box_type::none && c.below == box_type::none) {
      return horizontal_line(c.left, c.right);
    }

    // Corners
    if (c.above == box_type::none && c.left == box_type::none) {
      return nw_corner(c.below, c.right);
    } else if (c.above == box_type::none && c.right == box_type::none) {
      return ne_corner(c.left, c.below);
    } else if (c.below == box_type::none && c.left == box_type::none) {
      return sw_corner(c.right, c.above);
    }
    return se_corner(c.above, c.left);
  }
}
