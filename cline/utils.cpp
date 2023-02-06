#include <cstdlib>
#include <string_view>

#include <unistd.h>

namespace cline::utils {

int redirFD(int fd) {
    int fds[2];
    int res;

    res = pipe(fds);
    if (res == -1) return res;

    res = dup2(fds[1], fd);
    if (res == -1) return res;

    return fds[0];
}

int flushFD(int fd) {
    char buf[1024];
    int len;

    do{
        len = read(fd, buf, sizeof(buf));
        if(len < 0) return len;
    }while(len == sizeof(buf));

    return len;
}

void clearFD(int fd) {
    uint64_t buf;
    read(fd, &buf, sizeof(buf));
}

int countThreads(int fd) {
    char buf[256];
    int len = pread(fd, buf, sizeof(buf), 0);
    if(len == -1) return -1;
    buf[len] = 0;
    return atoi(buf);
}

[[noreturn]] void lastWords(int fd, std::string_view msg) {
    write(fd, msg.data(), msg.size());
    exit(EXIT_FAILURE);
}

} // namespace cline
