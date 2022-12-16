#pragma once

namespace j::mem {
  enum class return_allocation_tag { v };
  enum class keep_allocation_tag { v };
  constexpr inline return_allocation_tag return_allocation = return_allocation_tag::v;
  constexpr inline keep_allocation_tag keep_allocation = keep_allocation_tag::v;
}
