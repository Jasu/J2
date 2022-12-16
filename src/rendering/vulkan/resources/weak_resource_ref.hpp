#pragma once

#include "rendering/vulkan/resources/resource_ref.hpp"

namespace j::rendering::vulkan::resources {
  template<typename Resource, typename Description, typename ResourceDefinition>
  class weak_resource_ref : public detail::resource_ref_weak_base {
  public:
    using ref_t J_NO_DEBUG_TYPE = resource_ref<Resource, Description, ResourceDefinition>;

    constexpr weak_resource_ref() noexcept = default;

    weak_resource_ref(weak_resource_ref && rhs) noexcept = default;

    weak_resource_ref & operator=(weak_resource_ref && rhs) noexcept;

    weak_resource_ref(const weak_resource_ref & ref) noexcept;

    explicit weak_resource_ref(const ref_t & ref) noexcept;

    weak_resource_ref & operator=(const weak_resource_ref & rhs) noexcept;

    weak_resource_ref & operator=(const ref_t & rhs) noexcept;

    ref_t lock() const noexcept;

    ~weak_resource_ref();
  private:
    void unreference_record() noexcept;
  };

  template<typename Resource, typename Description, typename Def>
  weak_resource_ref<Resource, Description, Def> & weak_resource_ref<Resource, Description, Def>::operator=(weak_resource_ref && rhs) noexcept
  {
    if (wrapper != rhs.wrapper) {
      unreference_record();
      wrapper = rhs.wrapper;
      rhs.wrapper = nullptr;
    }
    return *this;
  }


  template<typename Resource, typename Description, typename Def>
  void weak_resource_ref<Resource, Description, Def>::unreference_record() noexcept {
    if (!wrapper) {
      return;
    }
    --wrapper->weak_refcount;
    Def::maybe_finalize_resource_wrapper(wrapper);
    wrapper = nullptr;
  }

  template<typename Resource, typename Description, typename Def>
  J_ALWAYS_INLINE weak_resource_ref<Resource, Description, Def>::~weak_resource_ref() {
    unreference_record();
  }

  template<typename Resource, typename Description, typename Def>
  weak_resource_ref<Resource, Description, Def>::weak_resource_ref(const ref_t & ref) noexcept
    : resource_ref_weak_base(ref.wrapper)
  {
    if (wrapper) {
      ++wrapper->weak_refcount;
    }
  }

  template<typename Resource, typename Description, typename Def>
  weak_resource_ref<Resource, Description, Def>::weak_resource_ref(const weak_resource_ref & ref) noexcept
    : resource_ref_weak_base(ref ? ref.wrapper : nullptr)
  {
    if (wrapper) {
      ++wrapper->weak_refcount;
    }
  }

  template<typename Resource, typename Description, typename Def>
  weak_resource_ref<Resource, Description, Def> & weak_resource_ref<Resource, Description, Def>::operator=(const weak_resource_ref & rhs) noexcept {
    if (&rhs != this) {
      unreference_record();
      if (rhs) {
        wrapper = rhs.wrapper;
        ++wrapper->weak_refcount;
      }
    }
    return *this;
  }

  template<typename Resource, typename Description, typename Def>
  weak_resource_ref<Resource, Description, Def> & weak_resource_ref<Resource, Description, Def>::operator=(const ref_t & rhs) noexcept {
    unreference_record();
    if (rhs) {
      wrapper = rhs.wrapper;
      ++wrapper->weak_refcount;
    }
    return *this;
  }

  template<typename Resource, typename Description, typename Def>
  resource_ref<Resource, Description, Def> weak_resource_ref<Resource, Description, Def>::lock() const noexcept {
    if (!*this) {
      return resource_ref<Resource, Description, Def>{};
    }
    return resource_ref<Resource, Description, Def>(wrapper);
  }
}
