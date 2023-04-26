#ifndef AXIO_BUFREADER
#define AXIO_BUFREADER

#include <string_view>

#include <unistd.h>

#include "emitter.h"

namespace axio {

    template <std::size_t S>
    class bufReader {
      public:
        bufReader() {}

        std::string_view read(Emitter& emitter) {
            int size = ::read(emitter.getRawFd(), buf_, S);
            return std::string_view(size < 0 ? nullptr : buf_, size);
        }

      private:
        char buf_[S];
    };
} // namespace axio

#endif // AXIO_BUFREADER
