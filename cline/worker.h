#pragma once

#include <pthread.h>

#include "./sysexcept.h"

namespace cline {

class Worker {
  public:
    Worker();

    bool notJoined() const noexcept { return !joined_; }
    bool isAlive();

    bool cancel();

    template <class T>
    void spawn(T* callable) {
        int rc = pthread_create(&worker_, NULL, Worker::work<T>, callable);
        /* can only be EAGAIN (Insufficient resources) */
        if(rc != 0) throw SySExcept("Thread creation failed", -rc);
        joined_ = false;
    }

    template <class T>
    static void* work(void* arg) {
        return (static_cast<T*>(arg)->*(&T::operator()))(), nullptr;
    }

  private:
    pthread_t worker_;
    bool      joined_;
};

} // namespace cline
