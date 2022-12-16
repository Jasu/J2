#include "properties/access_by_path.hpp"

#include "properties/wrappers/wrapper.hpp"
#include "properties/access/list_access.hpp"
#include "properties/access/map_access.hpp"
#include "properties/classes/object_access.hpp"
#include "properties/path.hpp"
#include "properties/visiting/visit_path.hpp"
#include "strings/string.hpp"

namespace j::properties {
  namespace {
    wrappers::wrapper get_default_value(const wrappers::wrapper_definition & def) {
      auto selected_type = def.possible_contents()[0];
      for (auto type : def.possible_contents()) {
        if (type->is_nil()) {
          selected_type = type;
        }
      }
      return selected_type->create_default();
    }

    template<typename Component>
    void access_array_index(wrappers::wrapper & wrapper, const Component & c, access_by_path_flags flags) {
      J_REQUIRE(wrapper.is_list(), "Path expects value to be a list, but it is not.");
      auto index = c.as_array_index();
      auto list = wrapper.as_list();
      auto sz = list.size();
      if (index < sz) {
        wrapper = list.at(index);
      } else {
        J_REQUIRE(flags.has(access_by_path_flag::create), "List index not found.");
        wrappers::wrapper default_value_ptr = get_default_value(list.item_definition());
        access::typed_access default_value = default_value_ptr.get_value();
        for (sz_t i = sz; i <= index; ++i) {
          list.push_back(default_value);
        }
        wrapper = list.at(index);
      }
    }

    template<typename Component>
    void access_map_key(wrappers::wrapper & wrapper, const Component & c, access_by_path_flags flags) {
      if (wrapper.is_object() && flags.has(access_by_path_flag::properties_as_keys)) {
        wrapper = wrapper.as_object().get_property(c.as_map_key());
        return;
      }

      J_REQUIRE(wrapper.is_map(), "Path expects value to be a map, but it is not.");
      auto map = wrapper.as_map();
      if (!flags.has(access_by_path_flag::create)) {
        wrapper = map.at(c.as_map_key());
      } else {
        auto it = map.find(c.as_map_key());
        if (it != map.end()) {
          wrapper = it->value();
        } else {
          wrappers::wrapper default_value_ptr = get_default_value(map.value_definition());
          wrapper = map.insert(c.as_map_key(), default_value_ptr.get_value()).first->value();
        }
      }
    }

    template<typename Component>
    void access_property(wrappers::wrapper & wrapper, const Component & c) {
      J_REQUIRE(wrapper.is_object(), "Path expects value to be an object, but it is not.");
      wrapper = wrapper.as_object().get_property(c.as_property_name());
    }
  }

  wrappers::wrapper access_by_path(wrappers::wrapper wrapper, const path & p, access_by_path_flags flags) {
    for (auto & c : p) {
      switch (c.type()) {
      case COMPONENT_TYPE::EMPTY:
        J_THROW("Unexpected empty component in path.");
      case COMPONENT_TYPE::ARRAY_INDEX:
        access_array_index(wrapper, c, flags);
        break;
      case COMPONENT_TYPE::MAP_KEY:
        access_map_key(wrapper, c, flags);
        break;
      case COMPONENT_TYPE::PROPERTY_NAME:
        access_property(wrapper, c);
        break;
      }
    }
    return wrapper;
  }

  wrappers::wrapper access_by_path(wrappers::wrapper wrapper, const visiting::visit_path & p, access_by_path_flags flags) {
    if (p.parent() && p.parent()->type() != COMPONENT_TYPE::EMPTY) {
      wrapper = access_by_path(wrapper, *p.parent(), flags);
    }
    switch (p.type()) {
    case COMPONENT_TYPE::EMPTY:
      break;
    case COMPONENT_TYPE::ARRAY_INDEX:
      access_array_index(wrapper, p, flags);
      break;
    case COMPONENT_TYPE::MAP_KEY:
      access_map_key(wrapper, p, flags);
      break;
    case COMPONENT_TYPE::PROPERTY_NAME:
      access_property(wrapper, p);
      break;
    }
    return wrapper;
  }
}
