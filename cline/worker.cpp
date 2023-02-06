#include <ctime>

#include "./worker.h"

namespace cline {
Worker::Worker(): joined_(true) {}

bool Worker::isAlive() {
    int rc = pthread_tryjoin_np(worker_, NULL);
    if(rc == EBUSY) return true;
    if(rc != 0) throw SySExcept("Error when joining worker thread", -rc);

    joined_ = true;
    return false;
}

bool Worker::cancel() {
    pthread_cancel(worker_);
    /* The worker thread may not cooperate with cancelling.
       Do timed join to avoid waiting indefinitely */
    timespec timeout = {0};
    clock_gettime(CLOCK_REALTIME, &timeout);
    timeout.tv_sec += 3;
    if(pthread_timedjoin_np(worker_, NULL, &timeout) != 0) return false;
    return joined_ = true;
}

} // namespace cline
