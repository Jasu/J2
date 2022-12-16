#pragma once

#include "exceptions/assert_lite.hpp"
#include "mem/pool.hpp"

namespace j::rendering::vulkan::buffers {
  template<typename> class suballocated_buffer;

  struct generic_suballocation final {
    uptr_t m_buffer = 0;
    u32_t offset = 0;
    u32_t size = 0;
    uptr_t userdata = 0;

    template<typename Ref>
    J_INLINE_GETTER Ref & buffer() noexcept {
      J_ASSUME(m_buffer != 0);
      return *reinterpret_cast<Ref *>(&m_buffer);
    }

    template<typename Ref>
    void set_buffer(typename Ref::resource_t * J_NOT_NULL buf) {
      J_ASSUME(m_buffer == 0);
      ::new (reinterpret_cast<Ref*>(&m_buffer)) Ref();
      *reinterpret_cast<Ref *>(&m_buffer) = Ref::from_this(buf);
    }

    template<typename Ref>
    void destroy_buffer() noexcept {
      J_ASSUME(m_buffer != 0);
      buffer<Ref>().~Ref();
      m_buffer = 0;
    }
  };

}
extern template class j::mem::pool<j::rendering::vulkan::buffers::generic_suballocation>;

namespace j::rendering::vulkan::buffers {
  using suballocation_pool_t = mem::pool<generic_suballocation>;
  extern suballocation_pool_t g_suballocation_pool;

  template<typename Ref>
  class suballocation {
  public:
    /// Construct an invalid suballocation.
    J_ALWAYS_INLINE constexpr suballocation() noexcept = default;

    /// Construct a valid suballocation.
    J_ALWAYS_INLINE suballocation(generic_suballocation * J_NOT_NULL generic) noexcept
      : m_generic_suballocation(generic)
    {
      J_ASSUME(generic->size != 0);
    }

    J_ALWAYS_INLINE explicit operator bool() const noexcept
    { return m_generic_suballocation; }

    J_ALWAYS_INLINE bool operator!() const noexcept
    { return !m_generic_suballocation; }

    J_INLINE_GETTER u32_t size() const noexcept {
      J_ASSUME_NOT_NULL(m_generic_suballocation);
      J_ASSUME(m_generic_suballocation->size != 0);
      return m_generic_suballocation->size;
    }

    J_INLINE_GETTER u32_t offset() const noexcept {
      J_ASSUME_NOT_NULL(m_generic_suballocation);
      return m_generic_suballocation->offset;
    }

    J_INLINE_GETTER const Ref & buffer() const noexcept {
      J_ASSUME_NOT_NULL(m_generic_suballocation);
      J_ASSUME(m_generic_suballocation->m_buffer != 0);
      return m_generic_suballocation->buffer<Ref>();
    }


    /// Free the transfer operation source in [m_buffer].
    void release() noexcept {
      J_ASSUME_NOT_NULL(m_generic_suballocation);
      static_cast<suballocated_buffer<Ref>&>(buffer().get()).m_allocator.free(offset(), size());
      m_generic_suballocation->destroy_buffer<Ref>();
      g_suballocation_pool.free(m_generic_suballocation);
      m_generic_suballocation = nullptr;
    }

    /// Get a pointer to the memory region to of the buffer.
    J_RETURNS_NONNULL u8_t * target() noexcept {
      return buffer().get().buffer_data() + offset();
    }

    uptr_t & userdata() const {
      J_ASSUME_NOT_NULL(m_generic_suballocation);
      return m_generic_suballocation->userdata;
    }

    J_INLINE_GETTER generic_suballocation * inner() const noexcept {
      return m_generic_suballocation;
    }
  private:
    generic_suballocation * m_generic_suballocation = nullptr;
  };
}
