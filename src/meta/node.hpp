#pragma once

#include "meta/attrs.hpp"
#include "meta/comments.hpp"

namespace j::meta {
  struct node {
    strings::string name;
    node_type type = node_none;
    u8_t index = 0U;
    doc_comment comment;
    attrs attrs;

    J_A(AI,ND) inline constexpr node() noexcept = default;
    J_A(AI,ND) inline constexpr explicit node(node_type type) noexcept : type(type) { }

    J_A(AI,ND) inline node(node_type type, strings::string && name) noexcept
      : name(static_cast<strings::string &&>(name)),
        type(type)
    {
    }

    node(node_type type, strings::string && name, doc_comment && comment) noexcept;

    J_A(AI,ND) inline node(node_type type, strings::const_string_view name) noexcept
    : name(name),
      type(type)
    { }

    node(node_type type, strings::const_string_view name, doc_comment && comment) noexcept;
  };
}
