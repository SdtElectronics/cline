#include <sys/un.h>
#include <unistd.h>

#include "./dispatcher.h"
#include "./socket.h"

namespace axio {
    Socket::Socket(Dispatcher& parent, uint32_t offset, int fd):
        Emitter(parent, offset, fd) {}

    bool Socket::connect(const Address& addr) const noexcept {
        int err = ::connect(fd_, (sockaddr*)&addr.addr, addr.len);
        return err == 0;
    }

    bool Socket::listen(const Address& addr, int wl) const noexcept {
        if(0 != (bind(fd_,(sockaddr*)&addr.addr, addr.len))) {
            return false;
        }

        return ::listen(fd_, wl) == 0;
    }

    bool Socket::accept(Socket::Client& client) const {
        auto addr = (sockaddr*)&client.addr;
        socklen_t len = sizeof(client.addr);
        client.fd = ::accept(fd_, addr, &len);
        return client.fd != -1;
    }

    int Socket::send(std::string_view buffer) const noexcept {
        return write(fd_, buffer);
    }

    int Socket::send(std::initializer_list<Iov> svs) const noexcept {
        return write(fd_, svs);
    }

    Socket::UNIXAddress::UNIXAddress(std::string_view address)
        : Address() {
        len = sizeof(sockaddr_un);
        sockaddr_un& servaddr = *((sockaddr_un*)&addr);
        servaddr.sun_family = AF_UNIX;

        if(address.size() > sizeof(servaddr.sun_path)) {
            throw std::out_of_range("Address is too long.");
        }
        address.copy(servaddr.sun_path, address.size());
    }

    Socket::UNIXAddress::UNIXAddress(const UNIXAddress& src) noexcept {
        sockaddr_un& dstaddr = *((sockaddr_un*)&addr);
        sockaddr_un& srcaddr = *((sockaddr_un*)&src.addr);
        dstaddr = srcaddr;
        len = src.len;
    }

    Socket::UNIXAddress&
    Socket::UNIXAddress::operator=(const UNIXAddress& src) noexcept {
        sockaddr_un& dstaddr = *((sockaddr_un*)&addr);
        sockaddr_un& srcaddr = *((sockaddr_un*)&src.addr);
        dstaddr = srcaddr;
        len = src.len;
        return *this;
    }

    Socket::UNIXAddressOwning::UNIXAddressOwning(std::string_view address)
        : UNIXAddress(address) {}

    Socket::UNIXAddressOwning::UNIXAddressOwning(UNIXAddressOwning&& src)
        noexcept: UNIXAddress(src) {
        ((sockaddr_un*)&src.addr)->sun_path[0] = '\0';
    }

    Socket::UNIXAddressOwning&
    Socket::UNIXAddressOwning::operator=(UNIXAddressOwning&& src) noexcept {
        UNIXAddress::operator=(src);
        ((sockaddr_un*)&src.addr)->sun_path[0] = '\0';
        return *this;
    }

    Socket::UNIXAddressOwning::~UNIXAddressOwning() noexcept {
        sockaddr_un& servaddr = *((sockaddr_un*)&addr);
        unlink(servaddr.sun_path);
    }


    int Socket::construct(Domain doamin, Type type) {
        int fd = socket(doamin, type, 0);
        if(fd < 0) throw InitError<Socket>();
        return fd;
    }

    int Socket::construct(const Client& client) noexcept {
        return client.fd;
    }
}
