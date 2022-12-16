#include "node.hpp"
#include "exceptions/assert.hpp"
#include "meta/term.hpp"
#include "meta/attr_context.hpp"
#include "meta/value_types.hpp"

namespace j::meta {
  namespace s = strings;

  node::node(node_type type, s::string && name, doc_comment && comment) noexcept
    : name(static_cast<strings::string &&>(name)),
      type(type),
      comment{static_cast<doc_comment &&>(comment)}
  { }

  node::node(node_type type, s::const_string_view name, doc_comment && comment) noexcept
    : name(name),
      type(type),
      comment(static_cast<doc_comment &&>(comment))
  { }
}
