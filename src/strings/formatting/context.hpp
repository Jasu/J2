#pragma once

#include "attributes/basic_operations.hpp"
#include "util/context_stack.hpp"

namespace j::strings::inline formatting {
  enum class context_type : u8_t {
    unspecified = 0,
    visual,
    visual_short,
    serialization,
  };
}

namespace j::strings::attributes {
  namespace a = j::attributes;

  inline constexpr a::attribute_definition is_multiline{
    a::value_type = type<bool>,
    a::tag = type<struct is_multiline_tag>};

  inline constexpr a::attribute_definition context_type{
    a::value_type = type<formatting::context_type>,
    a::tag = type<formatting::context_type>};

  inline constexpr a::attribute_definition indent{
    a::value_type = type<u16_t>,
    a::tag = type<struct indent_tag>};
}

namespace j::strings::inline formatting {
  struct formatting_context_t final {
    enum context_type context_type = context_type::unspecified;
    bool is_multiline = false;
    u16_t indent = 0U;

    J_BOILERPLATE(formatting_context_t, CTOR_NE_ND, COPY_NE_ND, MOVE_NE_ND)

    explicit formatting_context_t(const formatting_context_t * parent) noexcept;

    template<typename... Attrs>
    J_A(AI,ND) inline explicit formatting_context_t(const formatting_context_t * parent, Attrs && ... attrs) noexcept
      : formatting_context_t(*parent)
    {
      namespace sa = j::strings::attributes;
      namespace a = j::attributes;
      if constexpr (a::has<Attrs...>(sa::context_type)) {
        context_type = sa::context_type.get(static_cast<Attrs &&>(attrs)...);
      }
      if constexpr (a::has<Attrs...>(sa::indent)) {
        indent += sa::indent.get(static_cast<Attrs &&>(attrs)...);
      }
      if constexpr (a::has<Attrs...>(sa::is_multiline)) {
        is_multiline = sa::is_multiline.get(static_cast<Attrs &&>(attrs)...);
      }
    }
  };

  extern thread_local j::util::context_stack<formatting_context_t, true> formatting_context J_A(ND);
}
