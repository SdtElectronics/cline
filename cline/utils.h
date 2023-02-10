#include <string_view>

#include <sys/uio.h>

namespace cline::utils {

int redirFD(int fd);

void clearFD(int fd);

int flushFD(int fd);

int countThreads(int fd);

template <typename... SV>
int vecWrite(int fd, SV... msgs) {
    iovec vec[sizeof...(SV)];
    iovec *it = vec;

    for(const auto msg: { msgs... }) {
        it->iov_base = const_cast<char*>(msg.data());
        it->iov_len = msg.size();
        ++it;
    }

    return writev(fd, vec, sizeof...(SV));
}

}
