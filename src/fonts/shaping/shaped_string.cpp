#include "fonts/shaping/shaped_string.hpp"
#include "hzd/mem.hpp"
#include "mem/shared_ptr.hpp"

namespace j::fonts::shaping {
  shaped_string::shaped_string(u32_t size, u16_t font_index, u16_t num_uniques) noexcept
    : m_size(size),
      m_font_index(font_index),
      m_num_uniques(num_uniques)
  {
    J_ASSERT_NOT_NULL(m_size, m_num_uniques);
  }

  mem::shared_ptr<shaped_string> shaped_string::allocate(u32_t size, u16_t font_index, u16_t num_uniques) {
    J_ASSERT_NOT_NULL(size, num_uniques);
    return mem::make_shared_sized<shaped_string>(
      sizeof(shaped_string)
      + size * sizeof(positioned_glyph)
      + num_uniques * sizeof(u32_t),

      size, font_index, num_uniques);
  }
}
