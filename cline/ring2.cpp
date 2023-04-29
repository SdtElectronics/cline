#include "./ring2.h"

namespace cline {


Ring2::Ring2(std::size_t capacity): capacity_(capacity), size_(0) {}

void Ring2::reserve(std::size_t capacity) {
    pribuf_.reserve(capacity);
    secbuf_.reserve(capacity);
}

void Ring2::append(std::string_view sv) {
    append(sv.data(), sv.size());
}

void Ring2::append(const char* begin, std::size_t len) {
    pribuf_.append(begin, begin + len);
    if((++size_) == capacity_) {
        secbuf_.clear();
        secbuf_.swap(pribuf_);
        size_ = 0;
    }
}

std::string_view Ring2::head() const noexcept {
    return pribuf_;
}

std::string_view Ring2::tail() const noexcept {
    return secbuf_;
}

} // namespace cline
