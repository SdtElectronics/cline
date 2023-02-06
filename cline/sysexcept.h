#include <stdexcept>

namespace cline {

class SySExcept: public std::runtime_error {
  public:   // TODO: include info of the syscall
    SySExcept(const char* msg, int errcode);

    int getErr() const noexcept;

  private:
    const int errcode_;
};

} // namespace cline
