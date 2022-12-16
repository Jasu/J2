#include "debug/timing.hpp"

#include "containers/unsorted_string_map.hpp"
#include "exceptions/assert_lite.hpp"
#include "services/services.hpp"
#include "strings/find.hpp"
#include "strings/string_algo.hpp"
#include "strings/formatting/formatted_sink.hpp"

namespace j::debug {
  namespace c = containers;
  namespace {
    struct J_TYPE_HIDDEN timer_rec {
      i64_t last_value;
      timer * timer;
    };

    auto & get_timers() {
      static c::unsorted_string_map<c::unsorted_string_map<timer_rec>> timers;
      return timers;
    }
  }

  void register_timers(u32_t num, timer * const * timers,
                       const char * group_name, const char * names_) {
    J_ASSERT_NOT_NULL(num, timers, group_name, names_);
    auto & group = get_timers()[group_name];
    strings::const_string_view names{names_};
    for (u32_t i = 0; i < num; ++i) {
      J_ASSERT_NOT_NULL(timers[i]);
      names = strings::ltrim(names);
      i32_t pos = find_char(names, ',');
      strings::const_string_view name = names.slice(0, pos);
      name = strings::rtrim(name);
      J_ASSERT((pos < 0) == (i == num - 1), "Counter names mismatched.");
      group.insert(strings::string(name), timer_rec{ timers[i]->sum_ts, timers[i] });
      names.remove_prefix(pos + 1U);
    }
  }

  void dump_timers() {
    auto sink = services::service_container->get<strings::formatted_sink>(
      services::service_reference("stderr"));
    J_ASSERT_NOT_NULL(sink);
    bool has_printed_header = false;
    for (auto & group : get_timers()) {
      bool has_printed_group_header = false;
      for (auto & timer : *group.second) {
        const i64_t val = timer.second->timer->sum_ts;
        if (val == 0) {
          continue;
        }
        if (!has_printed_group_header) {
          sink->write("{} {#green_bg,white,bold}  {}:  {/}\n", has_printed_header ? "" : "\n", *group.first);
          has_printed_header = has_printed_group_header = true;
        }
        sink->write("   {#bright_cyan,bold}{}:{/} {#bold}{}{/}\n",
                    *timer.first, convert_timer_delta(val / timer.second->timer->count, time::time_unit::us));
        timer.second->timer->count = 0U;
        timer.second->timer->sum_ts = 0;
        timer.second->last_value = val;
      }
    }
    if (has_printed_header) {
      sink->write("\n");
    }
  }
}
