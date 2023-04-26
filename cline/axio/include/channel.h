#ifndef AXIO_PIPE
#define AXIO_PIPE

#include <cerrno>

#include <poll.h>

#include "./emitter.h"
#include "./write.h"

namespace axio {
    class Channel{
      public:
        Channel(int flags);
        int fds[2];
    };

    class ChanTX: public Emitter {
      public:
        enum Events: EvtVec {
            writable = POLLOUT,
            error    = POLLERR
        };
        enum InitErr { ioError = EIO };

        using Emitter::Emitter;

        ChanTX(int fd, int flags = 0);

        int send(std::string_view buffer) const noexcept;
        int send(std::initializer_list<Iov> svs) const noexcept;

        static int construct(Channel chan) noexcept;
        static int construct(int fd, int flags);

        static constexpr EvtVec defEvts = writable;
    };

    class ChanRX: public Emitter {
      public:
        enum Events: EvtVec {
            readable = POLLIN,
            error    = POLLERR
        };
        enum InitErr { ioError = EIO };

        using Emitter::Emitter;

        static int construct(Channel chan) noexcept;
        static int construct(int fd, int flags = 0);

        static constexpr EvtVec defEvts = readable;
    };
} // namespace axio

#endif // AXIO_PIPE
