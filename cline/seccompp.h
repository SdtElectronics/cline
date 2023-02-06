#pragma once

#include <initializer_list>

#include <seccomp.h>

namespace cline {

class Seccompp{
  public:
    Seccompp() noexcept;
    Seccompp(uint32_t defAction);
    Seccompp(Seccompp&& other) noexcept;

    Seccompp& operator=(Seccompp&& other) noexcept;

    Seccompp&& addRule(uint32_t action, int syscall, std::initializer_list<scmp_arg_cmp> arg = {});

    void swap(Seccompp& other) noexcept;

    void load();

    ~Seccompp();

  private:
    scmp_filter_ctx ctx_;
};

} // namespace cline
