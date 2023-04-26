#include <unistd.h>

#include "./write.h"

namespace axio {
    Iov::Iov(std::string_view sv) {
        iov_base = const_cast<char*>(sv.data());
        iov_len = sv.size();
    }

    int write(int fd, std::string_view sv) {
        return ::write(fd, sv.data(), sv.size());
    }

    int write(int fd, std::initializer_list<Iov> svs) {
        Iov* ptr = const_cast<Iov*>(std::data(svs));
        return writev(fd, ptr, svs.size());
    }

} // namespace axio
