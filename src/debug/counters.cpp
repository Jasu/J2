#include "debug/counters.hpp"

#include "containers/unsorted_string_map.hpp"
#include "exceptions/assert_lite.hpp"
#include "services/services.hpp"
#include "strings/find.hpp"
#include "strings/string_algo.hpp"
#include "strings/formatting/formatted_sink.hpp"

namespace j::debug {
  namespace c = containers;
  namespace {
    struct J_TYPE_HIDDEN counter_rec {
      const u32_t * counter;
      u32_t last_value;
      u32_t baseline_value;
    };

    auto & get_counters() {
      static c::unsorted_string_map<c::unsorted_string_map<counter_rec>> counters;
      return counters;
    }
  }

  void register_counters(u32_t num, const u32_t * const * counters,
                         const char * group_name, const char * names_) noexcept {
    J_ASSERT_NOT_NULL(num, counters, group_name, names_);
    auto & group = get_counters()[group_name];
    strings::const_string_view names{names_};
    for (u32_t i = 0; i < num; ++i) {
      J_ASSERT_NOT_NULL(counters[i]);
      names = strings::ltrim(names);
      i32_t pos = find_char(names, ',');
      strings::const_string_view name = names.slice(0, pos);
      name = strings::rtrim(name);
      J_ASSERT(pos < 0 == (i == num - 1U),
               "Counter names mismatched.");
      group.insert(strings::string(name), counter_rec{ counters[i], *counters[i], *counters[i] });
      names.remove_prefix(pos + 1U);
    }
  }

  void dump_counters(u32_t flags) {
    auto sink = services::service_container->get<strings::formatted_sink>(
      services::service_reference("stderr"));
    J_ASSERT_NOT_NULL(sink);
    bool has_printed_header = false;
    for (auto & group : get_counters()) {
      bool has_printed_group_header = false;
      for (auto & counter : *group.second) {
        const u32_t val = *counter.second->counter;
        if ((flags & only_changed) && counter.second->last_value == val) {
          continue;
        }
        if (!has_printed_group_header) {
          sink->write("{} {#green_bg,white,bold}  {}:  {/}\n", has_printed_header ? "" : "\n", *group.first);
          has_printed_header = has_printed_group_header = true;
        }
        sink->write("   {#bright_cyan,bold}{}:{/} {#bold}{}{/}\n",
                    *counter.first, (u32_t)(val - counter.second->baseline_value));
        counter.second->last_value = val;
        if (flags & accumulate) {
          counter.second->baseline_value = val;
        }
      }
    }
    if (has_printed_header) {
      sink->write("\n");
    }
  }
}
