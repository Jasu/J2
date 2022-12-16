#pragma once

#include "containers/vector.hpp"
#include "containers/trivial_array_fwd.hpp"
#include "mem/shared_ptr_fwd.hpp"

namespace j::fonts::shaping {
  class shaped_string;
}

J_DECLARE_EXTERN_VECTOR(j::mem::shared_ptr<j::fonts::shaping::shaped_string>);

namespace j::ui::inline text {
  using line_t = vector<mem::shared_ptr<fonts::shaping::shaped_string>>;

  struct positioned_string final {
    using nicely_copyable_tag_t = void;
    using zero_initializable_tag_t = void;

    /// Pointer to the string to be rendered.
    ///
    /// Owned by the physical line.
    fonts::shaping::shaped_string * string = nullptr;
    /// X position from the top of the line (i.e. always positive)
    float x = 0.0f;
    /// Y position from the left border of the line (i.e. always positive)
    float y = 0.0f;
  };
}

J_DECLARE_EXTERN_TRIVIAL_ARRAY_COPYABLE(j::ui::text::positioned_string);

namespace j::ui::inline text {
  using positioned_strings_t = j::trivial_array_copyable<positioned_string>;
  struct visual_line {
    /// Gap between lines.
    float gap = 0.0f;
    /// Height of the line, excluding gap.
    float height = 0.0f;
    positioned_strings_t words;
    using nicely_copyable_tag_t = void;
    using zero_initializable_tag_t = void;
  };
}

J_DECLARE_EXTERN_VECTOR(j::ui::text::visual_line);
