#include <unistd.h>

#include "./timer.h"

namespace cline {

Timer::Timer(): tfd_(-1) {}

int Timer::init() {
    return tfd_ = timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC);
}

int Timer::set(time_t sec) {
    itimerspec tim = {
        /* it_interval = */ { /* .tv_sec = */ 0, /* .tv_nsec = */ 0},
        /* .it_value   = */ { /* .tv_sec = */ sec, /* .tv_nsec = */ 0}
    };
    return timerfd_settime(tfd_, 0, &tim, nullptr);
}

int Timer::reset() {
    return timerfd_settime(tfd_, 0, &inf, nullptr);
}

Timer::~Timer() {
    close(tfd_);
}

} // namespace cline
