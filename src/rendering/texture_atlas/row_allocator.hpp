#pragma once

#include "containers/vector.hpp"
#include "geometry/rect.hpp"

namespace j::rendering::texture_atlas {
  struct span {
    using zero_initializable_tag_t = void;
    u16_t left = 0U;
    u16_t right = 0U;
  };
}

J_DECLARE_EXTERN_VECTOR(j::rendering::texture_atlas::span);

namespace j::rendering::texture_atlas {
  struct row {
    vector<span> free;
    u16_t height;
    u16_t max_free_width;
    using nicely_copyable_tag_t = void;
    using zero_initializable_tag_t = void;
  };
}

J_DECLARE_EXTERN_VECTOR(j::rendering::texture_atlas::row);

namespace j::rendering::texture_atlas {
  /// Allocator for texture atlas elements.
  struct row_allocator {
    row_allocator(u16_t width, u16_t height);

    geometry::rect_u16 allocate(geometry::vec2u16 size);

    void erase(geometry::rect_u16 rect);

    void dump() const;

    vector<row> m_rows;

    u16_t width, height;

    row * get_row(geometry::vec2u16 size, u16_t & top);
  };
}
