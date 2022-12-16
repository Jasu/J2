#pragma once

#include "exceptions/exceptions.hpp"

namespace j::windowing::x11 {
  extern const tags::tag_definition<u8_t> major_code;
  extern const tags::tag_definition<u16_t> minor_code;
  extern const tags::tag_definition<u8_t> error_code;
  extern const tags::tag_definition<u16_t> sequence;
  extern const tags::tag_definition<u32_t> resource_id;

  extern const tags::tag_definition<const char *> major_name;
  extern const tags::tag_definition<const char *> minor_name;
  extern const tags::tag_definition<const char *> error_name;
}
