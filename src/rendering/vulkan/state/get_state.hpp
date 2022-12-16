#pragma once

#include "rendering/vulkan/resources/resource_ref.hpp"

namespace j::rendering::vulkan::state {
  template<typename T>
  decltype(auto) get_state(T & ref) {
    return ref.wrapper->template resource_state<typename T::resource_definition_t::state_t>();
  }

}
