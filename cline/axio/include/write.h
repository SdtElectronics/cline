#ifndef AXIO_WRITE
#define AXIO_WRITE

#include <initializer_list>
#include <string_view>

#include <sys/uio.h>

namespace axio {
    struct Iov: public iovec {
        Iov(std::string_view sv);
    };

    int write(int fd, std::string_view sv);
    int write(int fd, std::initializer_list<Iov> svs);

} // namespace axio

#endif // AXIO_WRITE
