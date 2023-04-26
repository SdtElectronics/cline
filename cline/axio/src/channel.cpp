#include <unistd.h>

#include "./dispatcher.h"
#include "./channel.h"

namespace axio {
    Channel::Channel(int flags) {

    }

    ChanTX::ChanTX(int fd, int flags):
        Emitter(*static_cast<Dispatcher*>(nullptr), 0, 0) {
        fd_ = ChanTX::construct(fd, flags);
    }

    int ChanTX::send(std::string_view buffer) const noexcept {
        return write(fd_, buffer);
    }

    int ChanTX::send(std::initializer_list<Iov> svs) const noexcept {
        return write(fd_, svs);
    }

    int ChanTX::construct(Channel chan) noexcept {
        return chan.fds[0];
    }

    int ChanTX::construct(int fd, int flags = 0) {
        int fds[2];

        int rc = pipe2(fds, flags);
        if (rc == -1) throw InitError<ChanTX>();

        rc = dup2(fds[0], fd);
        if (rc == -1) throw InitError<ChanTX>();

        return fds[1];
    }


    int ChanRX::construct(Channel chan) noexcept {
        return chan.fds[1];
    }

    int ChanRX::construct(int fd, int flags) {
        int fds[2];

        int rc = pipe(fds);
        if (rc == -1) throw InitError<ChanRX>();

        rc = dup2(fds[1], fd);
        if (rc == -1) throw InitError<ChanRX>();

        return fds[0];
    }

} // namespace axio
