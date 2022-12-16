#include "time/hrtime.hpp"
#include "exceptions/exceptions.hpp"
#include "hzd/mem.hpp"

#include <linux/perf_event.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <cerrno>
#include <sys/mman.h>

namespace j::time {
  namespace {
    math::s32_32 get_tsc_to_ns_multiplier() {
      static math::s32_32 multiplier;
      if (multiplier.raw()) {
        return multiplier;
      }

      perf_event_attr attr;
      ::j::memzero(&attr, sizeof(attr));
      attr.type = PERF_TYPE_SOFTWARE;
      attr.size = sizeof(perf_event_attr);
      attr.config = PERF_COUNT_SW_DUMMY;
      attr.exclude_kernel = 1;
      attr.exclude_hv = 1;
      attr.exclude_idle = 1;
      attr.freq = 1;
      attr.exclude_callchain_kernel = 1;


      int fd = syscall(SYS_perf_event_open,
                       &attr,
                       (pid_t)0, (int)-1, (int)-1,
                       (unsigned long)(PERF_FLAG_FD_CLOEXEC));
      if (fd < 0) {
        J_THROW(system_error_exception() << message("perf_event_open: failed") << err_errno(errno));
      }
      void * page = ::mmap(nullptr, J_PAGE_SIZE * 2U, PROT_READ, MAP_SHARED, fd, 0U);
      if (page == MAP_FAILED) {
        close(fd);
        J_THROW(system_error_exception() << message("perf_event_open: mmap failed") << err_errno(errno));
      }

      const perf_event_mmap_page * mmap_data = (const perf_event_mmap_page*)page;
      multiplier = math::s32_32(mmap_data->time_mult << (32 - mmap_data->time_shift), math::raw_tag);
      return multiplier;
    }
  }

  math::s32_32 convert_timer_delta(i64_t timer, time_unit unit) noexcept {
    return get_tsc_to_ns_multiplier() / time_unit_factors[(u8_t)time_unit::ns][(u8_t)unit] * timer;
  }
}
