#ifndef AXIO_TIMER
#define AXIO_TIMER

#include <cerrno>

#include <poll.h>
#include <sys/timerfd.h>

#include "./emitter.h"

namespace axio {

class Timer: public Emitter {
  public:
    enum Events: EvtVec {
      fired = POLLIN,
      error = POLLERR
    };
    enum InitErr {
        unpermitted = EPERM,
        outofMemory = ENOMEM
    };
    enum ClockId {
        realtime = CLOCK_REALTIME,
        monotonic = CLOCK_MONOTONIC
    };

    Timer(Dispatcher& parent, uint32_t offset, int fd);

    int setTimeout(time_t sec, time_t nsec = 0) noexcept;
    int setInterval(time_t sec, time_t nsec = 0) noexcept;
    int clear();
    int reset();

    static int construct(ClockId id);

    static constexpr EvtVec defEvts = fired;

  private:
    static constexpr itimerspec inf = {
        /* it_interval = */ { /* .tv_sec = */ 0, /* .tv_nsec = */ 0},
        /* .it_value   = */ { /* .tv_sec = */ 0, /* .tv_nsec = */ 0}
    };
};

} // namespace axio

#endif // AXIO_TIMER
