#pragma once

#include "hzd/types.hpp"

namespace j::services::detail {

  struct finalizer {
    using fn_ptr_t = void (finalizer::*)() noexcept;
    fn_ptr_t fn_ptr;
    void (*fn)(fn_ptr_t, void*) noexcept;
  };

  template<typename Service>
  void finalizer_wrapper(finalizer::fn_ptr_t data, void * service) noexcept {
    auto fnptr = reinterpret_cast<void (Service::*)(void) noexcept>(data);
    (reinterpret_cast<Service*>(service)->*fnptr)();
  }

  class finalizer_call_set {
    finalizer * m_finalizers;
    u32_t m_size = 0;
  public:
    explicit finalizer_call_set(u32_t size)
      : m_finalizers(size ? ::new finalizer[size] : nullptr)
    {
    }

    ~finalizer_call_set() {
      ::delete[] m_finalizers;
    }

    template<typename Service>
    void add_finalizer(void (Service::*fn)()) noexcept {
      m_finalizers[m_size++] = finalizer{
        reinterpret_cast<finalizer::fn_ptr_t>(fn),
        &finalizer_wrapper<Service>
      };
    }

    void operator()(void * service) const noexcept {
      for (u32_t i = 0; i < m_size; ++i) {
        m_finalizers[i].fn(m_finalizers[i].fn_ptr, service);
      }
    }
  };
}
