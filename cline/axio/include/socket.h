#ifndef AXIO_SOCKET
#define AXIO_SOCKET

#include <cerrno>

#include <poll.h>
#include <sys/socket.h>

#include "./emitter.h"
#include "./write.h"

namespace axio {

class Socket: public Emitter {
  public:
    enum Events: EvtVec {
        readable     = POLLIN,
        acceptable   = POLLIN,
        writable     = POLLOUT,
        disconnected = POLLHUP,
        error        = POLLERR
    };
    enum InitErr {
        unpermitted = EPERM,
        outofMemory = ENOMEM
    };
    enum Domain {
        UNIX = AF_UNIX,
        IPV4 = AF_INET,
        IPV6 = AF_INET6
    };
    enum Type {
        stream   = SOCK_STREAM,
        datagram = SOCK_DGRAM,
        packet   = SOCK_SEQPACKET
    };

    struct Address {
        sockaddr_storage addr;
        socklen_t len;
    };

    struct UNIXAddress: public Address {
        UNIXAddress(std::string_view address);
        UNIXAddress(const UNIXAddress& src) noexcept;

        UNIXAddress& operator=(const UNIXAddress& src) noexcept;
    };

    struct UNIXAddressOwning: public UNIXAddress {
        UNIXAddressOwning(std::string_view address);
        UNIXAddressOwning(UNIXAddressOwning&& src) noexcept;

        UNIXAddressOwning& operator=(UNIXAddressOwning&& src) noexcept;

        ~UNIXAddressOwning() noexcept;
    };

    struct Client {
        int fd;
        sockaddr_storage addr;
    };

    Socket(Dispatcher& parent, uint32_t offset, int fd);

    bool connect(const Address& addr) const noexcept;
    bool listen(const Address& addr, int wl) const noexcept;
    bool accept(Client& clinet) const;

    int send(std::string_view buffer) const noexcept;
    int send(std::initializer_list<Iov> svs) const noexcept;

    static Address unixAddr(std::string_view addr);

    static int construct(Domain doamin, Type type);
    static int construct(const Client& client) noexcept;

    static constexpr EvtVec defEvts = readable;
};

} // namespace axio

#endif // AXIO_SOCKET
