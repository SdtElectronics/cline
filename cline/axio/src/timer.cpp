#include <unistd.h>

#include "./dispatcher.h"
#include "./timer.h"

namespace axio {

Timer::Timer(Dispatcher& parent, uint32_t offset, int fd):
    Emitter(parent, offset, fd) {}

int Timer::construct(ClockId id) {
    int fd = timerfd_create(id, TFD_CLOEXEC);
    if(fd < 0) throw InitError<Timer>();
    return fd;
}

int Timer::setTimeout(time_t sec, time_t nsec) noexcept {
    itimerspec tim = {
        /* it_interval = */ { /* .tv_sec = */ 0, /* .tv_nsec = */ 0},
        /* .it_value   = */ { /* .tv_sec = */ sec, /* .tv_nsec = */ nsec}
    };
    return timerfd_settime(fd_, 0, &tim, nullptr);
}

int Timer::setInterval(time_t sec, time_t nsec) noexcept {
    itimerspec tim = {
        /* it_interval = */ { /* .tv_sec = */ sec, /* .tv_nsec = */ nsec},
        /* .it_value   = */ { /* .tv_sec = */ sec, /* .tv_nsec = */ nsec}
    };
    return timerfd_settime(fd_, 0, &tim, nullptr);
}

int Timer::clear() {
    uint64_t buf;
    read(fd_, &buf, sizeof(buf));
    return buf;
}

int Timer::reset() {
    return timerfd_settime(fd_, 0, &inf, nullptr);
}

} // namespace axio
