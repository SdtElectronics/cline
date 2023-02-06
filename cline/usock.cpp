#include <cstring>

#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <sys/un.h>
#include <unistd.h>

#include "usock.h"

namespace cline {

USock::USock(): fd_(-1) {}

int USock::bind(const char* address) {
    int rc = socket(AF_UNIX, SOCK_SEQPACKET, 0);
    if(rc < 0) return rc;

    struct sockaddr_un servaddr = {0};
    servaddr.sun_family = AF_UNIX;

    if (strlen(address) > sizeof(servaddr.sun_path)) return -1;
    strcpy(servaddr.sun_path, address);

    int err = connect(rc, (struct sockaddr *)&servaddr, sizeof(struct sockaddr_un));
    if(err != 0) return err;

    return fd_ = rc;
}

int USock::send(std::string_view buffer) const noexcept {
    return write(fd_, buffer.data(), buffer.size());
}

int USock::send(std::string_view header, std::string_view body) const noexcept  {
    struct iovec frame[2];
    frame[0].iov_base = const_cast<char*>(header.data());
    frame[0].iov_len = header.size();
    frame[1].iov_base = const_cast<char*>(body.data());
    frame[1].iov_len = body.size();

    return writev(fd_, frame, 2);
}

void USock::end() {
    close(fd_);
}

USock::~USock() {
    close(fd_);
}

} // namespace cline
