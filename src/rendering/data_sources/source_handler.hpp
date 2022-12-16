#pragma once

#include "exceptions/assert_lite.hpp"

namespace j::strings {
  class string;
}

namespace j::rendering::vulkan::inline context {
  struct render_context;
}

namespace j::rendering::data_sources {
  template<typename KeyT, typename InfoT, typename BufferViewT>
  class source_handler {
  public:
    J_ALWAYS_INLINE constexpr source_handler() noexcept = default;

    J_ALWAYS_INLINE explicit constexpr source_handler(u8_t index) noexcept
      : m_index(index)
    { J_ASSUME(index < 16); }

    virtual ~source_handler() { }

    virtual u64_t get_userdata(KeyT source) const = 0;

    virtual InfoT get_info(KeyT source) const = 0;

    virtual strings::string describe(KeyT source) const = 0;

    virtual void copy_to(const vulkan::render_context & context,
                         BufferViewT & to, KeyT source) const = 0;

    J_INLINE_GETTER bool has_index() const noexcept {
      return m_index != 255U;
    }

    J_INLINE_GETTER u32_t index() const noexcept {
      return m_index;
    }

    J_ALWAYS_INLINE void set_index(u8_t index) noexcept {
      J_ASSUME(m_index == 255U);
      J_ASSUME(index < 16);
      m_index = index;
    }

    source_handler(const source_handler &) = delete;
    source_handler(source_handler &&) = delete;
    source_handler & operator=(const source_handler &) = delete;
    source_handler & operator=(source_handler &&) = delete;
  private:
    u32_t m_index = 255U;
  };

}
