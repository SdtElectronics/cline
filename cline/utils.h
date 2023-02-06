#include <string_view>

namespace cline::utils {

int redirFD(int fd);

void clearFD(int fd);

int flushFD(int fd);

int countThreads(int fd);

[[noreturn]] void lastWords(int fd, std::string_view msg);

}
