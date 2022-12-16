#pragma once

#include "attributes/attribute_definition.hpp"
#include "attributes/basic_operations.hpp"
#include "strings/parsing/parse.hpp"
#include "containers/vector.hpp"
#include "cli/argument_definition.hpp"
#include "cli/common_attributes.hpp"
#include "containers/trivial_array_fwd.hpp"

J_DECLARE_EXTERN_VECTOR(j::strings::const_string_view);

namespace j::services {
  struct injection_context;
}

namespace j::cli {
  namespace a = attributes;
  /// Attribute specifying the callback for a command.
  inline constexpr a::attribute_definition is_required{
    a::tag = type<struct is_required>,
    a::is_flag};

  inline constexpr a::attribute_definition default_value{
    a::tag = type<struct default_value_tag>};

  namespace detail {
    vector<strings::const_string_view> get_argument_as_strings(
      const argument_definition & definition,
      const services::injection_context * ic);

    template<typename T>
    struct argument_parser final {
      static inline T parse_argument(vector<strings::const_string_view> && args) {
        return strings::parse_fn<T>::parse(args[0]);
      }
    };

    template<typename T>
    struct argument_parser<vector<T>> final {
      static inline vector<T> parse_argument(vector<strings::const_string_view> && args) {
        vector<T> result(args.size());
        for (auto & v : args) {
          result.emplace_back(strings::parse_fn<T>::parse(v));
        }
        return result;
      }
    };

    template<typename T>
    struct argument_parser<noncopyable_vector<T>> final {
      static inline noncopyable_vector<T> parse_argument(vector<strings::const_string_view> && args) {
        noncopyable_vector<T> result(args.size());
        for (auto & v : args) {
          result.emplace_back(strings::parse_fn<T>::parse(v));
        }
        return result;
      }
    };

    template<typename T>
    struct argument_parser<trivial_array<T>> final {
      static inline trivial_array<T> parse_argument(vector<strings::const_string_view> && args) {
        trivial_array<T> result(containers::uninitialized, args.size());
        for (auto & v : args) {
          result.initialize_element(strings::parse_fn<T>::parse(v));
        }
        return result;
      }
    };

    template<>
    struct argument_parser<vector<strings::const_string_view>> final {
      J_INLINE_GETTER static vector<strings::const_string_view> && parse_argument(vector<strings::const_string_view> && args) {
        return static_cast<vector<strings::const_string_view> &&>(args);
      }
    };


    template<typename T, typename DefaultT>
    class argument final {
    public:
      inline constexpr static bool has_dependencies_v = false;
      using cli_argument_tag J_NO_DEBUG_TYPE = void;

      static constexpr inline bool has(void *, const void *) noexcept {
        // Has default value, so is always available.
        return true;
      }

      inline decltype(auto) get(void *, const services::injection_context * ic, void *) const {
        vector<strings::const_string_view> args = get_argument_as_strings(definition, ic);
        if (args.empty()) {
          return T(default_value);
        }
        return argument_parser<T>::parse_argument(static_cast<vector<strings::const_string_view> &&>(args));
      }

      strings::const_string_view spec;
      DefaultT default_value;
      argument_definition definition;
    };

    template<typename T>
    class argument<T, void> final {
    public:
      inline constexpr static bool has_dependencies_v = false;
      using cli_argument_tag J_NO_DEBUG_TYPE = void;

      J_INLINE_GETTER static constexpr bool has(void *, const void *) noexcept {
        // TODO
        return true;
      }

      J_INLINE_GETTER decltype(auto) get(void *, const services::injection_context * ic, void *) const {
        return argument_parser<T>::parse_argument(get_argument_as_strings(definition, ic));
      }

      strings::const_string_view spec;
      argument_definition definition;
    };

    template<>
    class argument<bool, void> final {
    public:
      inline constexpr static bool has_dependencies_v = false;
      using cli_argument_tag J_NO_DEBUG_TYPE = void;

      J_INLINE_GETTER static constexpr bool has(void *, const void *) noexcept {
        return true;
      }

      J_INLINE_GETTER bool get(void *, const services::injection_context * ic, void *) const {
        auto strings = get_argument_as_strings(definition, ic);
        if (!definition.has_value) {
          return !strings.empty();
        }
        return argument_parser<bool>::parse_argument(static_cast<vector<strings::const_string_view> &&>(strings));
      }

      strings::const_string_view spec;
      argument_definition definition;
    };

    template<typename DefaultT>
    struct argument_selector final {
      strings::const_string_view spec;
      argument_definition definition;
      DefaultT default_value;

      template<typename Arg>
      J_INLINE_GETTER constexpr auto select(u32_t index) const noexcept {
        auto a = argument<j::remove_cref_t<Arg>, DefaultT>{spec, default_value, definition};
        a.definition.index = index;
        return a;
      }
    };

    template<>
    struct argument_selector<void> final {
      strings::const_string_view spec;
      argument_definition definition;

      template<typename Arg>
      J_INLINE_GETTER constexpr auto select(u32_t index) const noexcept {
        auto a = argument<j::remove_cref_t<Arg>, void>{spec, definition};
        a.definition.index = index;
        return a;
      }
    };

    template<typename DefaultT>
    explicit argument_selector(strings::const_string_view, argument_definition, const DefaultT &) -> argument_selector<DefaultT>;

    explicit argument_selector(strings::const_string_view, argument_definition) -> argument_selector<void>;
  }

  template<typename... Args>
  J_INLINE_GETTER constexpr auto option(strings::const_string_view spec, Args && ... args) noexcept {
    constexpr bool is_multiple = j::attributes::has<Args...>(a::is_multiple);
    const char * short_desc = nullptr, * desc = nullptr;
    if constexpr(j::attributes::has<Args...>(short_description)) {
      short_desc = short_description.get(static_cast<Args &&>(args)...);
    }
    if constexpr(j::attributes::has<Args...>(description)) {
      desc = description.get(static_cast<Args &&>(args)...);
    }
    if constexpr (j::attributes::has<Args...>(default_value)) {
      return detail::argument_selector{
        spec,
        {
          .is_option = true,
          .is_multiple = is_multiple,
          .has_value = true,
          .short_description = short_desc,
          .long_description = desc
        },
        default_value.get(static_cast<Args &&>(args)...)
      };
    } else {
      return detail::argument_selector{
        spec,
        {
          .is_option = true,
          .is_multiple = is_multiple,
          .has_value = true,
          .short_description = short_desc,
          .long_description = desc,
        }
      };
    }
  }

  template<typename... Args>
  J_INLINE_GETTER constexpr auto argument(strings::const_string_view name, Args && ... args) noexcept {
    constexpr bool is_multiple = j::attributes::has<Args...>(a::is_multiple);
    const char * short_desc = nullptr, * desc = nullptr;
    if constexpr(j::attributes::has<Args...>(short_description)) {
      short_desc = short_description.get(static_cast<Args &&>(args)...);
    }
    if constexpr(j::attributes::has<Args...>(description)) {
      desc = description.get(static_cast<Args &&>(args)...);
    }
    if constexpr (j::attributes::has<Args...>(default_value)) {
      return detail::argument_selector{
        name,
        {
          .is_multiple = is_multiple,
          .has_value = true,
          .short_description = short_desc,
          .long_description = desc
        },
        default_value.get(static_cast<Args &&>(args)...)};
    } else {
      return detail::argument_selector{
        name,
        {
          .is_multiple = is_multiple,
          .is_required = j::attributes::has<Args...>(j::cli::is_required),
          .has_value = true,
          .short_description = short_desc,
          .long_description = desc
        }};
    }
  }

  J_INLINE_GETTER consteval auto argument(strings::const_string_view name) noexcept {
    return detail::argument_selector{name, {
        .is_multiple = false,
        .is_required = false,
        .has_value = true,
        .short_description = nullptr,
        .long_description = nullptr,
      }};
  }

  template<typename... Args>
  J_INLINE_GETTER consteval auto flag(strings::const_string_view name,  Args && ... args) noexcept {
    const char * short_desc = nullptr, * desc = nullptr;
    if constexpr(j::attributes::has<Args...>(short_description)) {
      short_desc = short_description.get(static_cast<Args &&>(args)...);
    }
    if constexpr(j::attributes::has<Args...>(description)) {
      desc = description.get(static_cast<Args &&>(args)...);
    }
    return detail::argument_selector{name, {
        .is_option = true,
        .is_multiple = false,
        .is_required = false,
        .has_value = false,
        .short_description = short_desc,
        .long_description = desc,
      }};
  }
}
