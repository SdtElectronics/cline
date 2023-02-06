#include <utility>

#include "./seccompp.h"
#include "./sysexcept.h"

namespace cline {

Seccompp::Seccompp() noexcept: ctx_(nullptr) {}

Seccompp::Seccompp(Seccompp&& other) noexcept: ctx_(nullptr) {
    this->swap(other);
}

Seccompp::Seccompp(uint32_t defAction) {
    ctx_ = seccomp_init(defAction);
    if(ctx_ == nullptr) {
        throw SySExcept("seccomp context init failed", -1);
    }
}

Seccompp& Seccompp::operator=(Seccompp&& other) noexcept {
    this->swap(other);
    return *this;
}

void Seccompp::swap(Seccompp& other) noexcept {
    std::swap(ctx_, other.ctx_);
}

Seccompp&& Seccompp::addRule(uint32_t action, int syscall, std::initializer_list<scmp_arg_cmp> arg) {
    int res = seccomp_rule_add_array(ctx_, action, syscall, arg.size(), std::data(arg));
    if(res != 0) throw SySExcept("add seccomp rule failed", -res);

    return std::move(*this);
}

void Seccompp::load() {
    int res = seccomp_load(ctx_);
    if(res < 0) {
        throw SySExcept("load seccomp filter failed", -res);
    }
}

Seccompp::~Seccompp() {
    if(ctx_ != nullptr) seccomp_release(ctx_);
}

} // namespace cline
