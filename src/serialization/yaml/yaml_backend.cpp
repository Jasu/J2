#include "serialization/yaml/yaml_backend.hpp"

#include "files/paths/path.hpp"
#include "properties/wrappers/stored_wrapper.hpp"
#include "files/memory_mapping.hpp"
#include "services/service_definition.hpp"
#include "mem/shared_ptr.hpp"
#include "serialization/yaml/value_wrapper.hpp"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wextra-semi-stmt"
#include <c4/yml/parse.hpp>
#pragma clang diagnostic pop

namespace j::serialization::yaml {
  namespace detail {
    namespace {
      /// Container for the root node, holding the allocated rapidyaml structures.
      struct J_HIDDEN yaml_root_data {
        J_NO_DEBUG J_ALWAYS_INLINE explicit yaml_root_data(c4::yml::Tree && t) noexcept
        : tree(static_cast<c4::yml::Tree &&>(t)),
        node{&tree, tree.root_id()}
        {
        }

        c4::yml::Tree tree;
        yaml_wrapper_data node;
      };
    }
  }

  bool yaml_backend::supports(const files::path & path) const {
    auto ext = path.extension();
    return ext == "yaml" || ext == "yml";
  }

  properties::wrappers::stored_wrapper yaml_backend::read(const files::path & path) const {
    files::memory_mapping mapping = files::memory_mapping(path, files::map_full_range);
    // Wrap the memory-mapped buffer into c4's own string view.
    const c4::csubstr ryml_buffer(mapping.data(), mapping.size());
    // File name to pass to rapidyaml, for diagnostics only.
    const c4::csubstr ryml_path(path.as_string().data(), path.as_string().size());

    // Rapidyaml returns Tree by value, move it immediately into a
    // shared_ptr-managed object, that holds both the pointer and the property
    // wrapper for the root object.
    mem::shared_ptr<detail::yaml_root_data> root_data(mem::make_shared<detail::yaml_root_data>(c4::yml::parse(ryml_path, ryml_buffer)));

    // Resolve aliases in the YAML document(s).
    root_data->tree.resolve();

    // Note that a YAML variant is always returned, even if the specific type
    // of the value is known. This simplifies things, and leaves doors open for
    // implementing mutable properties.
    return properties::wrappers::stored_wrapper{
      properties::wrappers::wrapper{yaml::detail::yaml_variant_wrapper_definition, &root_data->node},
      static_cast<mem::shared_ptr<detail::yaml_root_data> &&>(root_data)
    };
  }

  namespace {
    namespace s = services;
    s::service_definition<yaml_backend> def(
      "serialization.backend.yaml",
      "YAML serialization backend",
      s::implements = s::interface<backend>,
      s::global_singleton
    );
  }
}
