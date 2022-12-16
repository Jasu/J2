#include "rendering/images/image_buffer_copy.hpp"
#include "hzd/string.hpp"

namespace j::rendering::images::detail {
  void copy_direct(
    u8_t * to,
    const u8_t * from,
    u16_t width_bytes,
    u16_t height
  ) noexcept {
    J_ASSERT_NOT_NULL(to, from, width_bytes, height);
    memcpy(to, from, (u32_t)width_bytes * height);
  }

  void copy_strided(
    u8_t * to,
    const u8_t * from,
    u16_t width_bytes,
    u16_t height,
    i16_t to_stride,
    i16_t from_stride
  ) noexcept {
    while (height--) {
      memcpy(to, from, width_bytes);
      to += to_stride;
      from += from_stride;
    }
  }

  void copy_convert(
    u8_t * to,
    const u8_t * from,
    u16_t width,
    u16_t height,
    i16_t to_stride,
    i16_t from_stride,
    image_format to_format,
    image_format from_format
  ) {
    J_ASSERT(to_format != image_format::gray8, "To format cannot be grayscale.");
    from_stride -= bytes_per_pixel(from_format) * width;
    to_stride -= bytes_per_pixel(to_format) * width;
    while (height--) {
      for (u16_t x = 0; x < width; ++x) {
        u8_t r, g, b, a = 255;

        switch (from_format) {
        case image_format::rgb24:
          r = *from++;
          g = *from++;
          b = *from++;
          break;
        case image_format::bgr24:
          b = *from++;
          g = *from++;
          r = *from++;
          break;
        case image_format::rgba32:
          r = *from++;
          g = *from++;
          b = *from++;
          a = *from++;
          break;
        case image_format::bgra32:
          b = *from++;
          g = *from++;
          r = *from++;
          a = *from++;
          break;
        case image_format::gray8:
          r = g = b = *from++;
          break;
        }

        switch (to_format) {
        case image_format::rgb24:
          *to++ = r;
          *to++ = g;
          *to++ = b;
          break;
        case image_format::bgr24:
          *to++ = b;
          *to++ = g;
          *to++ = r;
          break;
        case image_format::rgba32:
          *to++ = r;
          *to++ = g;
          *to++ = b;
          *to++ = a;
          break;
        case image_format::bgra32:
          *to++ = b;
          *to++ = g;
          *to++ = r;
          *to++ = a;
          break;
        case image_format::gray8:
          J_THROW("Target format cannot be gray.");
        }
      }
      to += to_stride;
      from += from_stride;
    }
  }
}
