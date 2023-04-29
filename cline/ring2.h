#pragma once

#include <string>

namespace cline {

class Ring2 {
  public:
    Ring2(std::size_t capacity);

    void reserve(std::size_t capacity);

    void append(std::string_view sv);
    void append(const char* begin, std::size_t len);

    std::string_view head() const noexcept;

    std::string_view tail() const noexcept;

  private:
    std::string pribuf_;
    std::string secbuf_;
    std::size_t capacity_;
    std::size_t size_;
};

} // namespace cline
