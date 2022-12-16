#pragma once

#include "properties/access.hpp"
#include "properties/visiting/visit_definition_path.hpp"

namespace j::properties::visiting {
  namespace detail {
    template<typename Visitor, typename... TailArgs>
    void visit_definition_recursive_impl(Visitor && visitor,
                                         const visit_definition_path & path,
                                         TailArgs && ... tail_args)
    {
      static_cast<Visitor &&>(visitor)(path, static_cast<TailArgs &&>(tail_args)...);
      for (auto def : path.access_definitions()) {
        switch (def->type) {
        case PROPERTY_TYPE::LIST: {
          if (path.path_contains(def->as_list())) {
            break;
          }
          const visit_definition_path child_path = path.enter_list_item(def->as_list());
          visit_definition_recursive_impl(static_cast<Visitor &&>(visitor), child_path, static_cast<TailArgs &&>(tail_args)...);
          break;
        }
        case PROPERTY_TYPE::MAP: {
          auto & map = def->as_map();
          if (path.path_contains(map)) {
            break;
          }
          const visit_definition_path key_path = path.enter_map_key(map);
          const visit_definition_path value_path = path.enter_map_value(map);
          visit_definition_recursive_impl(static_cast<Visitor &&>(visitor), key_path, static_cast<TailArgs &&>(tail_args)...);
          visit_definition_recursive_impl(static_cast<Visitor &&>(visitor), value_path, static_cast<TailArgs &&>(tail_args)...);
          break;
        }
        case PROPERTY_TYPE::SET: {
          if (path.path_contains(def->as_set())) {
            break;
          }
          const visit_definition_path child_path = path.enter_set_item(def->as_set());
          visit_definition_recursive_impl(static_cast<Visitor &&>(visitor), child_path, static_cast<TailArgs &&>(tail_args)...);
          break;
        }
        case PROPERTY_TYPE::OBJECT: {
          auto & object = def->as_object();
          for (auto & prop : object.get_properties()) {
            if (!path.path_contains(*prop.second)) {
              const visit_definition_path prop_path = path.enter_object_property(*prop.second);
              visit_definition_recursive_impl(static_cast<Visitor &&>(visitor), prop_path, static_cast<TailArgs &&>(tail_args)...);
            }
          }
          break;
        }
        default:
          break;
        }
      }
    }
  }

  template<typename Visitor, typename... TailArgs>
  void visit_definition_recursive(const wrappers::wrapper_definition & definition,
                                  Visitor && visitor,
                                  TailArgs && ... tail_args)
  {
    const visit_definition_path path(definition);
    detail::visit_definition_recursive_impl(static_cast<Visitor &&>(visitor), path, static_cast<TailArgs &&>(tail_args)...);
  }

  template<typename Visitor, typename... TailArgs>
  void visit_definition_recursive(const access::typed_access_definition & definition,
                                  Visitor && visitor,
                                  TailArgs && ... tail_args)
  {
    auto def = &definition;
    const visit_definition_path path(def);
    detail::visit_definition_recursive_impl(static_cast<Visitor &&>(visitor), path, static_cast<TailArgs &&>(tail_args)...);
  }
}
