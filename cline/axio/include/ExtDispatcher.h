#ifndef AXIO_EXTDISPATCHER
#define AXIO_EXTDISPATCHER

#include <utility>

#include "./dispatcher.h"

namespace axio {
    class ExtDispatcher: public Dispatcher {
      public:
        inline ExtDispatcher(pollfd* begin):
            Dispatcher(begin, 0) {}

        inline virtual void drop(uint32_t offset) override {
            int& fd = base_[offset].fd;
            fd = -1;
        }

        template <typename Emitter, typename ...Args>
        Emitter track(Args&&... args) {
            int fd = Emitter::construct(std::forward<Args>(args)...);
            if(fd < 0) throw InitError<Emitter>();
            base_[size_].events = Emitter::defEvts;
            base_[size_].fd = fd;
            return Emitter(*this, size_++, fd);
        }
    };
} // namespace axio

#endif // AXIO_EXTDISPATCHER
