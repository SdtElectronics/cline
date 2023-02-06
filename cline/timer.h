#pragma once

#include <sys/timerfd.h>

namespace cline {

class Timer {
  public:
    Timer();

    Timer(Timer& other) = delete;
    Timer& operator=(Timer& other) = delete;

    int init();
    int set(time_t sec);
    int reset();

    ~Timer();

  private:
    int tfd_;

    static constexpr itimerspec inf = {
        /* it_interval = */ { /* .tv_sec = */ 0, /* .tv_nsec = */ 0},
        /* .it_value   = */ { /* .tv_sec = */ 0, /* .tv_nsec = */ 0}
    };
};

} // namespace cline
