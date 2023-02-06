#pragma once

#include <string_view>

namespace cline {

class USock {
  public:
    USock();

    USock(USock& other) = delete;
    USock& operator=(USock& other) = delete;

    int bind(const char* address);
    int send(std::string_view buffer) const noexcept;
    int send(std::string_view header, std::string_view body) const noexcept;
    void end();

    ~USock();

  private:
    int fd_;
};

} // namespace cline
