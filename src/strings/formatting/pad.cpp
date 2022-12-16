#include "strings/formatting/pad.hpp"
#include "streams/sink.hpp"

namespace j::strings::inline formatting {
  namespace {
    constexpr const char g_spaces[] =
      "                                                                "
      "                                                                "
      "                                                                "
      "                                                                ";
    constexpr const char g_zeroes[] =
      "0000000000000000000000000000000000000000000000000000000000000000"
      "0000000000000000000000000000000000000000000000000000000000000000"
      "0000000000000000000000000000000000000000000000000000000000000000"
      "0000000000000000000000000000000000000000000000000000000000000000";
    static_assert(sizeof(g_spaces) == 257);
    static_assert(sizeof(g_zeroes) == 257);
  }

  [[nodiscard]] const_string_view get_spaces(i32_t count) noexcept {
    J_ASSERT(count < 257);
    return {g_spaces + 256 - count, count};
  }

  [[nodiscard]] const_string_view get_zeroes(i32_t count) noexcept {
    J_ASSERT(count < 257);
    return {g_zeroes + 256 - count, count};
  }

  void write_spaces(streams::sink & target, i32_t count) {
    J_ASSERT(count < 256);
    J_ASSERT(count >= 0);
    while (count) {
      count -= target.write(g_spaces, ::j::min(256, count));
    }
  }

  void write_zeroes(streams::sink & target, u32_t count) {
    while (count) {
      count -= target.write(g_zeroes, ::j::min(256U, count));
    }
  }

  void write_left_padding(streams::sink & target, const pad_settings & pad_settings, u32_t length) {
    u32_t left_pad = pad_settings.pad_left;
    switch (pad_settings.alignment) {
    case alignment::left:
      break;
    case alignment::right:
      left_pad += ::j::max(0, (i32_t)pad_settings.pad_to - length);
      break;
    case alignment::center:
      left_pad += ::j::max(0, (i32_t)(pad_settings.pad_to - length) / 2);
      break;
    }
    pad_settings.zero ? write_zeroes(target, left_pad) : write_spaces(target, left_pad);
  }

  void write_right_padding(streams::sink & target, const pad_settings & pad_settings, u32_t length) {
    u32_t right_pad = pad_settings.pad_right;
    switch (pad_settings.alignment) {
    case alignment::left:
      right_pad += ::j::max(0, (i32_t)pad_settings.pad_to - length);
      break;
    case alignment::right:
      break;
    case alignment::center:
      right_pad += ::j::max(0, ((i32_t)pad_settings.pad_to - length + 1U) / 2);
      break;
    }
    pad_settings.zero ? write_zeroes(target, right_pad) : write_spaces(target, right_pad);
  }
}
