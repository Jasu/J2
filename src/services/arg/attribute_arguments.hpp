#pragma once

#include "services/arg/metadata_arguments.hpp"
#include "services/arg/service_argument.hpp"
#include "services/arg/default_value_wrapper.hpp"
#include "attributes/attributes.hpp"
#include "containers/vector.hpp"

namespace j::services::arg {
  namespace detail {
    template<typename T>
    inline constexpr bool is_any_v = j::is_same_v<T, mem::any>;

    template<typename Argument, typename AttrDef, typename MetadataArgument, typename Enable = void>
    class attribute_argument_impl;

    // Getting as the correct dynamic result type
    template<typename AttrDef, typename MetadataArgument>
    class attribute_argument_impl<typename AttrDef::value_t, AttrDef, MetadataArgument, void> {
    public:
      using result_type = const typename AttrDef::value_t &;
      inline constexpr static bool has_dependencies_v = false;

      [[nodiscard]] J_NO_DEBUG inline static bool has(services::container * c,
                                                      const services::injection_context * ic) noexcept
      {
        if (!MetadataArgument::has(c, ic)) {
          return false;
        }
        return MetadataArgument::get(c, ic, nullptr)->dynamic_metadata.template contains<AttrDef>();
      }

      [[nodiscard]] J_NO_DEBUG inline static decltype(auto) get(services::container * c,
                                          const services::injection_context * ic, void *)
      {
        return MetadataArgument::get(c, ic, nullptr)->dynamic_metadata
          .template first<AttrDef>().template get<typename AttrDef::value_t>();
      }
    };

    // Getting as mem::any
    template<typename AttrDef, typename MetadataArgument>
    class attribute_argument_impl<mem::any, AttrDef, MetadataArgument, void> {
    public:
      using result_type = mem::any &;
      inline constexpr static bool has_dependencies_v = false;

      [[nodiscard]] J_NO_DEBUG inline static bool has(services::container * c, const services::injection_context * ic) noexcept {
        if (!MetadataArgument::has(c, ic)) {
          return false;
        }
        return MetadataArgument::get(c, ic, nullptr)->dynamic_metadata.template contains<AttrDef>();
      }

      [[nodiscard]] J_NO_DEBUG inline static mem::any & get(services::container * c,
                                             const services::injection_context * ic,
                                             void *)
      {
        return MetadataArgument::get(c, ic, nullptr)->dynamic_metadata
          .template first<AttrDef>();
      }
    };


    // Converting vector<mem::any> to an vector<unrelated type>
    template<typename Argument, typename AttrDef, typename MetadataArgument>
    class attribute_argument_impl<vector<Argument>, AttrDef, MetadataArgument, j::enable_if_t<AttrDef::is_any_v && AttrDef::is_multiple_v && !is_any_v<j::remove_const_t<Argument>>>> {
    public:
      inline constexpr static bool has_dependencies_v = false;
      using result_type = vector<Argument>;

      [[nodiscard]] J_NO_DEBUG inline static bool has(services::container * c, const services::injection_context * ic) noexcept {
        if (!MetadataArgument::has(c, ic)) {
          return false;
        }
        auto s = MetadataArgument::get(c, ic, nullptr)->dynamic_metadata.template maybe_at<AttrDef>();
        if (!s) {
          return false;
        }
        for (auto & e : s) {
          if (!e.template is<j::remove_const_t<Argument>>()) {
            return false;
          }
        }
        return true;
      }

      J_INLINE_GETTER static auto get(
        services::container * c, const services::injection_context * ic, void *)
      {
        vector<Argument> result;
        for (auto & e : MetadataArgument::get(c, ic, nullptr)->dynamic_metadata.template maybe_at<AttrDef>()) {
          result.push_back(e->template get<j::remove_const_t<Argument>>());
        }
        return result;
      }
    };

    // Converting mem::any to an unrelated type
    template<typename Argument, typename AttrDef, typename MetadataArgument>
    class attribute_argument_impl<Argument, AttrDef, MetadataArgument, j::enable_if_t<AttrDef::is_any_v && !is_any_v<Argument>>> {
    public:
      using result_type = Argument &;
      inline constexpr static bool has_dependencies_v = false;

      J_INLINE_GETTER static bool has(services::container * c, const services::injection_context * ic) noexcept {
        if (!MetadataArgument::has(c, ic)) {
          return false;
        }
        auto & dyn = MetadataArgument::get(c, ic, nullptr)->dynamic_metadata;
        auto f = dyn.template maybe_first<AttrDef>();
        return f && f->template is<Argument>();
      }

      [[nodiscard]] inline static decltype(auto) get(services::container * c, const services::injection_context * ic, void *) {
        return MetadataArgument::get(c, ic, nullptr)->dynamic_metadata
          .template first<AttrDef>().template get<Argument>();
      }
    };


    // Service type
    template<typename Argument, typename AttrDef, typename MetadataArgument>
    class attribute_argument_impl<
      Argument,
      AttrDef,
      MetadataArgument,
      j::enable_if_t<j::is_same_v<typename AttrDef::value_t, services::service_reference>>> {
    public:
      using result_type = remove_ref_t<Argument>;
      inline constexpr static bool has_dependencies_v = service_argument<Argument>::has_dependencies_v;

      J_INLINE_GETTER static bool has(services::container * c,
                                const services::injection_context * ic) noexcept {
        if (!MetadataArgument::has(c, ic)) {
          return false;
        }
        auto any = MetadataArgument::get(c, ic, nullptr)->dynamic_metadata.template maybe_first<AttrDef>();
        return any && service_argument<Argument>{any->template get<services::service_reference>(), false}.has(c, ic);
      }

      J_INLINE_GETTER static decltype(auto) get(services::container * c,
                                                 const services::injection_context * ic,
                                                 services::detail::dependencies_t * deps)
      {
        return service_argument<Argument>{
          MetadataArgument::get(c, ic, nullptr)->dynamic_metadata.template first<AttrDef>().template get<services::service_reference>(), false
        }.get(c, ic, deps);
      }
    };


    // Service vector type
    template<typename Interface, typename AttrDef, typename MetadataArgument>
    class attribute_argument_impl<
      vector<Interface>,
      AttrDef,
      MetadataArgument,
      j::enable_if_t<
        j::is_same_v<typename AttrDef::value_t, vector<services::service_reference>>
      >> {
    public:
      using result_type = vector<Interface>;
      inline constexpr static bool has_dependencies_v = service_argument<Interface>::has_dependencies_v;

      J_INLINE_GETTER static bool has(services::container * c, const services::injection_context * ic) noexcept {
        if (!MetadataArgument::has(c, ic)) {
          return false;
        }
        auto s = MetadataArgument::get(c, ic, nullptr)->dynamic_metadata.template maybe_at<AttrDef>();
        if (!s) {
          return false;
        }
        for (auto & x : s) {
          // TODO: Make this configurable, i.e. allow only having some matches.
          if (!service_argument<Interface>{s.first->template get<Interface>(), false}.has(c, ic)) {
            return false;
          }
        }
        return true;
      }

      J_INLINE_GETTER static auto get(services::container * c, const services::injection_context * ic, services::detail::dependencies_t * deps) {
        auto & dyn = MetadataArgument::get(c, ic, deps)->dynamic_metadata;
        vector<Interface> result;
        for (auto & e : dyn.template maybe_at<AttrDef>()) {
          result.push_back(service_argument<Interface>{e->template get<Interface>(), false}.get(c, ic, deps));
        }
        return result;
      }
    };
  }

  namespace detail {
    template<typename AttrDef>
    struct service_attribute_selector {
      template<typename Argument>
      J_INLINE_GETTER_NO_DEBUG constexpr static auto select(u32_t) noexcept {
        return detail::attribute_argument_impl<j::remove_cref_t<Argument>, AttrDef, detail::service_metadata_argument_impl<const class_metadata *, injection_context_argument<const services::injection_context *>>>{};
      }
    };

    template<typename AttrDef>
    struct interface_attribute_selector {
      template<typename Argument>
      J_INLINE_GETTER_NO_DEBUG constexpr static auto select(u32_t) noexcept {
        return detail::attribute_argument_impl<j::remove_cref_t<Argument>, AttrDef, detail::interface_metadata_argument_impl<const class_metadata *, injection_context_argument<const services::injection_context *>>>{};
      }
    };

    template<typename AttrDef>
    struct parent_service_attribute_selector {
      template<typename Argument>
      J_INLINE_GETTER_NO_DEBUG constexpr static auto select(u32_t) noexcept {
        return detail::attribute_argument_impl<j::remove_cref_t<Argument>, AttrDef, detail::service_metadata_argument_impl<const class_metadata *, parent_injection_context_argument<const services::injection_context *>>>{};
      }
    };

    template<typename AttrDef>
    struct parent_interface_attribute_selector {
      template<typename Argument>
      J_INLINE_GETTER_NO_DEBUG constexpr static auto select(u32_t) noexcept {
        return detail::attribute_argument_impl<j::remove_cref_t<Argument>, AttrDef, detail::interface_metadata_argument_impl<const class_metadata *, parent_injection_context_argument<const services::injection_context *>>>{};
      }
    };
  }

  template<typename AttrDef, typename Arg>
  J_INLINE_GETTER_NO_DEBUG constexpr auto service_attribute(const AttrDef &, Arg && arg) noexcept {
    return detail::default_value_wrapper_selector{
      default_value.get(static_cast<Arg &&>(arg)),
      detail::service_attribute_selector<AttrDef>{},
    };
  }

  template<typename AttrDef>
  J_INLINE_GETTER_NO_DEBUG constexpr auto service_attribute(const AttrDef &) noexcept {
    return detail::service_attribute_selector<AttrDef>{};
  }

  template<typename AttrDef, typename Arg>
  J_INLINE_GETTER_NO_DEBUG constexpr auto interface_attribute(const AttrDef &, Arg && arg) noexcept {
    return detail::default_value_wrapper_selector{
      default_value.get(static_cast<Arg &&>(arg)),
      detail::interface_attribute_selector<AttrDef>{},
    };
  }

  template<typename AttrDef>
  J_INLINE_GETTER_NO_DEBUG constexpr auto interface_attribute(const AttrDef &) noexcept {
    return detail::interface_attribute_selector<AttrDef>{};
  }

  template<typename AttrDef, typename Arg>
  J_INLINE_GETTER_NO_DEBUG constexpr auto parent_service_attribute(const AttrDef &, Arg && arg) noexcept {
    return detail::default_value_wrapper_selector{
      default_value.get(static_cast<Arg &&>(arg)),
      detail::parent_service_attribute_selector<AttrDef>{},
    };
  }

  template<typename AttrDef>
  J_INLINE_GETTER_NO_DEBUG constexpr auto parent_service_attribute(const AttrDef &) noexcept {
    return detail::parent_service_attribute_selector<AttrDef>{};
  }

  template<typename AttrDef, typename Arg>
  J_INLINE_GETTER_NO_DEBUG constexpr auto parent_interface_attribute(const AttrDef &, Arg && arg) noexcept {
    return detail::default_value_wrapper_selector{
      default_value.get(static_cast<Arg &&>(arg)),
      detail::parent_interface_attribute_selector<AttrDef>{},
    };
  }

  template<typename AttrDef>
  J_INLINE_GETTER_NO_DEBUG constexpr auto parent_interface_attribute(const AttrDef &) noexcept {
    return detail::parent_interface_attribute_selector<AttrDef>{};
  }
}
