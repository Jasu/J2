#pragma once

#include "mem/shared_ptr_fwd.hpp"
#include "strings/string.hpp"
#include "containers/span.hpp"
#include "rendering/data_sources/source_handler.hpp"

namespace j::inline containers {
  template<typename T> class vector;
}

namespace j::rendering::data_sources {
  template<typename KeyT, typename InfoT, typename BufferViewT>
  class source_manager {
  public:
    J_INLINE_GETTER InfoT get_info(KeyT key) const {
      return handler(key).get_info(key);
    }

    J_INLINE_GETTER u64_t get_userdata(KeyT key) const {
      return handler(key).get_userdata(key);
    }

    J_INLINE_GETTER strings::string describe(KeyT key) const {
      return handler(key).describe(key);
    }

    J_ALWAYS_INLINE void copy_to(const vulkan::render_context & context,
                 BufferViewT & to,
                 KeyT key) const
    {
      J_ASSERT(to);
      handler(key).copy_to(context, to, key);
    }

    J_ALWAYS_INLINE constexpr source_manager() noexcept = default;

    source_manager(source_manager &&) = delete;
    source_manager(const source_manager &) = delete;
    source_manager & operator=(source_manager &&) = delete;
    source_manager & operator=(const source_manager &) = delete;
  protected:
    void initialize(u8_t start_index, vector<mem::shared_ptr<source_handler<KeyT, InfoT, BufferViewT>>> && handlers) noexcept {
      J_ASSERT(start_index + handlers.size() <= 16, "Index out of range.");
      for (auto & h : handlers) {
        J_ASSERT_NOT_NULL(h);
        h->set_index(start_index);
        m_handlers[start_index++] = static_cast<mem::shared_ptr<source_handler<KeyT, InfoT, BufferViewT>> &&>(h);
      }
    }

    source_handler<KeyT, InfoT, BufferViewT> & handler(KeyT key) const noexcept {
      J_ASSERT(key && key.handler_index() < 16 && m_handlers[key.handler_index()],
        "Invalid key");
      return *m_handlers[key.handler_index()];
    }

    mem::shared_ptr<source_handler<KeyT, InfoT, BufferViewT>> m_handlers[16];
  };

}
