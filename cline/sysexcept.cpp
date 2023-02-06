#include "./sysexcept.h"

namespace cline {

SySExcept::SySExcept(const char* msg, int errcode):
    std::runtime_error(msg), errcode_(errcode) {}

int SySExcept::getErr() const noexcept {
    return errcode_;
}

} // namespace cline
