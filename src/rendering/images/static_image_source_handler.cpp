#include "rendering/images/static_image_source_handler.hpp"

#include "rendering/images/image_buffer_copy.hpp"
#include "rendering/images/static_image_source.hpp"
#include "strings/format.hpp"

namespace j::rendering::images {
  static_image_source_handler static_image_source_handler::instance{};

  static_image_source_handler::static_image_source_handler()
    : image_source_handler(0)
  {
  }

  void static_image_source_handler::copy_to(const vulkan::render_context &,
                                            image_buffer_view & to, image_source_key source) const {
    J_ASSERT_NOT_NULL(to, source);
    J_ASSERT(source.handler_index() == 0, "Source has incorrect handler.");
    auto static_src = reinterpret_cast<static_image_source *>(source.uptr() & 0xFFFFFFFFFFFFULL);
    image_buffer_copy(to, *static_src);
  }

  strings::string static_image_source_handler::describe(image_source_key source) const {
    J_ASSERT(source.handler_index() == 0, "Source has incorrect handler.");
    return strings::format("Static image at 0x{:012X}", source.uptr() & 0xFFFFFFFFFFFFULL);
  }

  image_buffer_info static_image_source_handler::get_info(image_source_key source) const {
    J_ASSERT_NOT_NULL(source);
    J_ASSERT(source.handler_index() == 0, "Source has incorrect handler.");
    return reinterpret_cast<static_image_source *>(source.uptr() & 0xFFFFFFFFFFFFULL)->info;
  }
  u64_t static_image_source_handler::get_userdata(image_source_key) const {
    return 0ULL;
  }
}
