#include <unistd.h>

#include "./dispatcher.h"
#include "./emitter.h"

namespace axio {

    Emitter::Emitter(Dispatcher& parent, uint32_t offset, int fd):
        parent_(parent), offset_(offset), fd_(fd) {};

    Emitter::Emitter(const Emitter& other):
        parent_(other.parent_), offset_(other.offset_), fd_(other.fd_) {
    };

    Emitter& Emitter::operator=(Emitter& other) {
        other.offset_ = offset_;
        other.fd_ = fd_;
        return *this;
    }

    void Emitter::update(int fd) noexcept {
        parent_.base_[offset_].fd = fd;
        fd_ = fd;
    }

    void Emitter::listen(EvtVec events) noexcept {
        parent_.base_[offset_].events = events;
    }

    void Emitter::drop() noexcept {
        parent_.drop(offset_);
        close(fd_);
    }

    int Emitter::getRawFd() const noexcept {
        return fd_;
    }

} // namespace axio
