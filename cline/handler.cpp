#include <chrono>
#include <thread>

#include <fcntl.h>

#include "event.h"

#include "handler.h"

using namespace std::string_view_literals;
using namespace cline::protocol;

namespace cline {

Handler::Handler(int blockTime, int taskChk, Ring2& rec, cling::Interpreter& it):
    blockTime_(blockTime), taskChk_(taskChk), lineRec_(rec), dispatcher(fds),
    timer(dispatcher.track<axio::Timer>(axio::Timer::monotonic)),
    usock(dispatcher.track<axio::Socket>(axio::Socket::UNIX, axio::Socket::packet)),
    processor_(it, [sock = usock](MsgType hdr, std::string_view body) mutable {
        if(sock.send({ getMsgHdr(hdr), body }) == -1) {
            if(errno == EPIPE) {
                /* Client disconnected */

            } else {
                /* weird shit happened to the socket */
                /* better to just disconnect */
                sock.drop();
                // TODO: log this incident
            }
        } else if(hdr == mFATAL) {
            sock.drop();
        }
    }) {}

void Handler::startsvc(const char* unixaddr, int maxidle) {
    axio::Socket::UNIXAddress addr(unixaddr);
    if(!usock.connect(addr)) {
        throw svcError("Failed to establish connection with gateway\n");
    }
    if(usock.send(getMsgHdr(mHELLO)) < 0) {
        throw svcError("Failed to send handshake message\n");
    }

    auto loopo = dispatcher.track<axio::ChanRX>(STDOUT_FILENO, O_DIRECT);
    auto loope = dispatcher.track<axio::ChanRX>(STDERR_FILENO, O_DIRECT);

    std::array events = {
        axio::Event(axio::Timer ::fired,        timer, &Handler::timeout),
        axio::Event(axio::Socket::readable,     usock, &Handler::socketi),
        axio::Event(axio::ChanRX::readable,     loopo, &Handler::stdouti),
        axio::Event(axio::ChanRX::readable,     loope, &Handler::stderri),
        axio::Event(axio::Socket::disconnected, usock, &Handler::socketx),
        axio::Event(axio::Timer ::error,        timer, &Handler::pollerr),
        axio::Event(axio::ChanRX::error,        loopo, &Handler::pollerr),
        axio::Event(axio::ChanRX::error,        loope, &Handler::pollerr),
    };

    dispatcher.dispatch(
        events.begin(), events.end(), maxidle, &Handler::inactive, *this
    );

    loopo.drop();
    loope.drop();
}

bool Handler::inactive() {
    respond(mWARNING, "Inactive session expired"sv);
    return false;
}

bool Handler::timeout(axio::Emitter emitter) {
    timer.clear();
    if(worker.isAlive()) { /* timeout */
        if(!worker.cancel()) {
            respond(mFATAL, "Thread cancellation failed"sv);
            throw svcError("Thread cancellation failed\n");
        }
        respond(mTIMEOUT);
    } else {               /* joined */
        if(countThreads() != 1) {
            /* also check here in case a ghost thread is created
               and no further interpret request is received */
            respond(mFATAL, "Untracked thread(s) detected"sv);
            throw svcError("Untracked thread(s) detected\n");
        }
        respond(mJOINED);
    }

    return true;
}

bool Handler::socketi(axio::Emitter emitter) {
    static axio::ChanTX emulsin(STDIN_FILENO);
    std::string_view buf = read(emitter);
    if(!buf.data()) throw svcError("Failed to read data from socket\n");
    if(buf.size() == buflen) {
        respond(mREJECTED, "Invalid input: too long"sv);
        return true;
    }
    switch(buf[0]) {
        case rINTERPRET:
        if(worker.notJoined() && worker.isAlive()) {
            respond(mPROCESSING);
        } else {
            if(countThreads() != 1) {
                respond(mFATAL, "Untracked thread(s) detected"sv);
                throw svcError("Untracked thread(s) detected\n");
            }

            auto code = buf.substr(1);
            lineRec_.append(code);
            worker.spawn(processor_.process(code));
            timer.setTimeout(blockTime_);
        }
        break;

        case rEMULINPUT:
        emulsin.send(buf.substr(1));
        break;

        case rSOFTRESET:
        if(worker.notJoined()) {
            /* avoid flooding */
            std::this_thread::sleep_for(std::chrono::milliseconds(300));
            timer.reset();     // it can't fail so don't check the rc
            if(!worker.cancel()) {
                respond(mFATAL, "Thread cancellation failed"sv);
                throw svcError("Thread cancellation failed\n");
            }
            respond(mKILLED);
        } else {
            /* not in processing */
        }
        break;

        case rKEEPALIVE:
        /* Kill stale sessions & avoid flooding */
        if(constexpr unsigned int life = 60 /* m */; ++heartbeats > life) {
            respond(mLONG);
            return false;
        }
        respond(mOK);
        break;

        default:
        respond(mREJECTED, "Unknown request type"sv);
    }

    return true;
}

bool Handler::socketx(axio::Emitter emitter) {
    return false;
}

bool Handler::stdouti(axio::Emitter emitter) {
    std::string_view buf = read(emitter);
    if(!buf.data()) throw svcError("Failed to read data from stdout\n");
    if(buf.size() == buflen) {
        respond(mREJECTED,
                    "Too much data written to stdout. Discarding..."sv);
        return true;
    }

    respond(mSTDOUT, buf);
    return true;
}

bool Handler::stderri(axio::Emitter emitter) {
    std::string_view buf = read(emitter);
    if(!buf.data()) throw svcError("Failed to read data from stderr\n");
    if(buf.size() == buflen) {
        respond(mREJECTED,
                    "Too much data written to stderr. Discarding..."sv);
        return true;
    }

    respond(mSTDERR, buf);
    return true;
}

bool Handler::pollerr(axio::Emitter emitter) {
    throw svcError("poll received POLLERR\n");
    return false;
}

void Handler::respond(MsgType hdr) {
    if(usock.send(getMsgHdr(hdr)) == -1) {
        throw svcError("Failed to reach to the client\n");
    }
}

void Handler::respond(MsgType hdr, std::string_view body) {
    if(usock.send({ getMsgHdr(hdr), body }) == -1) {
        throw svcError("Failed to reach to the client\n");
    }
}

std::string_view Handler::read(axio::Emitter emitter) {
    /* When processor is working, it owns the primary buffer */
    Buf& recv = worker.notJoined() ? secondaryBuf : primaryBuf;
    return recv.read(emitter);
}

int Handler::countThreads() {
    char buf[256];
    int len = pread(taskChk_, buf, sizeof(buf), 0);
    if(len == -1) throw svcError("Failed to check threads\n");
    buf[len] = 0;
    return atoi(buf);
}

}
