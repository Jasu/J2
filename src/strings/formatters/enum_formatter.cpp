#include "strings/formatters/enum_formatter.hpp"
#include "containers/hash_map.hpp"
#include "containers/unsorted_string_map.hpp"
#include "strings/styling/styled_sink.hpp"
#include "strings/formatters/integers.hpp"

J_DEFINE_EXTERN_HASH_MAP(u64_t, j::strings::formatters::detail::enum_value_detail_set);
J_DEFINE_EXTERN_UNSORTED_STRING_MAP(u64_t);

namespace j::strings::formatters::detail {
  const enum_value_details & enum_value_detail_set::get_values(context_type t) const noexcept {
    switch (t) {
    case context_type::unspecified:
    case context_type::visual:
      if (visual.name) {
        return visual;
      }
      break;
    case context_type::visual_short:
      if (visual_short.name) {
        return visual_short;
      }
      break;
    case context_type::serialization:
      if (serialization.name) {
        return serialization;
      }
      break;
    }

    if (visual.name) {
      return visual;
    }
    if (visual_short.name) {
      return visual_short;
    }
    return serialization;
  }
  enum_formatter_base::enum_formatter_base(u32_t num) {
    m_values_by_name.reserve(num);
  }

  enum_formatter_base::enum_formatter_base(enum_value_span span) {
    initialize_serialization(span);
  }

  void enum_formatter_base::initialize_visual(enum_value_span values) {
    for (auto & v : values) {
      auto & set = m_enums.emplace(v.value).first->second;
      set.visual.name = v.name;
      set.visual.style = v.style;
    }
  }

  void enum_formatter_base::initialize_visual_short(enum_value_span values) {
    for (auto & v : values) {
      auto & set = m_enums.emplace(v.value).first->second;
      set.visual_short.name = v.name;
      set.visual_short.style = v.style;
    }
  }

  void enum_formatter_base::initialize_serialization(enum_value_span values) {
    m_values_by_name.reserve(values.size());
    for (auto & v : values) {
      add_value(v);
    }
  }

  void enum_formatter_base::add_value(const enum_value & value) {
    auto & set = m_enums.emplace(value.value).first->second;
    set.serialization.name = value.name;
    set.serialization.style = value.style;
    m_values_by_name.insert(value.name, (u64_t&&)value.value);
  }

  void enum_formatter_base::format(
    const const_string_view & format_options,
    u64_t value,
    styled_sink & target,
    style current_style,
    formatter_type type
  ) const {
    const auto t = formatting_context->context_type;
    if (J_LIKELY(type == formatter_type::normal) || !value) {
      auto e = m_enums.maybe_at(value);
      if (!J_LIKELY(e)) {
        if (value) {
          integer_formatter_v<u64_t>.do_format("", value, target, current_style);
        } else {
          target.write("NIL");
        }
        return;
      }
      auto & d = e->get_values(t);
      target.write_styled(d.style.empty() ? current_style : d.style, d.name);
    } else {
      bool is_first = true;
      for (auto & p : m_enums) {
        u64_t val = p.first;
        if (!val) {
          continue;
        }
        if (type == formatter_type::flags_shift) {
          J_ASSERT(val < 64U, "Too large flag shift value.");
          val = 1U << val;
        }
        if ((value & val) == val) {
          if (!is_first) {
            target.write("|", 1);
          }
          is_first = false;
          format(format_options, val, target, current_style, formatter_type::normal);
        }
      }
    }
  }

  u64_t enum_formatter_base::parse(const const_string_view & str) const {
    return m_values_by_name.at(str);
  }

  u32_t enum_formatter_base::get_length(
    const const_string_view &,
    const u64_t & value,
    formatter_type type
  ) const noexcept {
    const auto t = formatting_context->context_type;
    if (type == formatter_type::normal) {
      auto v = m_enums.maybe_at(value);
      if (!v) {
        if (value) {
          return integer_formatter_v<u64_t>.do_get_length("", value);
        } else {
          return 3;
        }
      }
      return v->get_values(t).name.size();
    }

    u32_t result = 0U;
    for (auto & p : m_enums) {
      u64_t val = p.first;
      if (!val) {
        continue;
      }
      if (type == formatter_type::flags_shift) {
        J_ASSERT(val < 64U, "Too large flag shift value.");
        val = 1U << val;
      }
      if ((value & val) == val) {
        result += p.second.get_values(t).name.size();
      }
    }
    return result;
  }
}
