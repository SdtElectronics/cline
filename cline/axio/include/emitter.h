#ifndef AXIO_EMITTER
#define AXIO_EMITTER

#include <cstdint>

namespace axio {
    using EvtVec = short;

    class Dispatcher;

    class Emitter {
      public:
        Emitter(Dispatcher& parent, uint32_t offset, int fd);

        Emitter(const Emitter& other);
        Emitter& operator=(Emitter& other);

        void update(int fd) noexcept;

        void listen(EvtVec events) noexcept;

        void drop() noexcept;

        int getRawFd() const noexcept;

        template<typename> friend class Event;

      protected:
        using Offset = uint32_t;

        Dispatcher& parent_;
        Offset      offset_;
        int         fd_;
    };
} // namespace axio

#endif // AXIO_EMITTER
