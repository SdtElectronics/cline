#ifndef AXIO_EVENT
#define AXIO_EVENT

#include "./emitter.h"

namespace axio {
    template <typename Callable>
    struct Event {
        Event(EvtVec evt, Emitter& emt, Callable cb):
            events(evt), offset(emt.offset_), callback(cb) {}

        using Offset = Emitter::Offset;

        EvtVec   events;
        Offset   offset;
        Callable callback;
    };
} // namespace axio

#endif // AXIO_EVENT
