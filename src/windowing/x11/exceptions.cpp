#include "windowing/x11/exceptions.hpp"

namespace j::windowing::x11 {
  const tags::tag_definition<u8_t> major_code{"Major code"};
  const tags::tag_definition<u16_t> minor_code{"Minor code"};
  const tags::tag_definition<u8_t> error_code{"Error code"};
  const tags::tag_definition<u16_t> sequence{"Sequence"};
  const tags::tag_definition<u32_t> resource_id{"Resource ID"};

  const tags::tag_definition<const char *> major_name{"Major name"};
  const tags::tag_definition<const char *> minor_name{"Minor name"};
  const tags::tag_definition<const char *> error_name{"Error name"};
}
