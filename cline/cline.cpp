#include <array>
#include <cerrno>
#include <chrono>
#include <csignal>
#include <cstdlib>
#include <thread>

#include <fcntl.h>
#include <poll.h>
#include <sys/prctl.h>

#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/Signals.h"
#include "llvm/Support/PrettyStackTrace.h"

#include "clang/Basic/LangOptions.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/FrontendTool/Utils.h"

#include "cling/Interpreter/Interpreter.h"

#include "./processor.h"
#include "./protocol.h"
#include "./timer.h"
#include "./usock.h"
#include "./utils.h"
#include "./worker.h"

using namespace cline::utils;
using namespace cline::protocol;
using namespace std::string_view_literals;

constexpr time_t blockTime = 10 /* s */;
constexpr int idleTime = 1000*60*10 /*10min*/;
enum Fds {tim = 0, msg, out, idCnt};

int main(int argc, char *argv[]) {
    if(prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0) != 0) /* unreachable */;

    const int efd = STDERR_FILENO;
    const int pfd = open(getenv("CLINE_TASKS"), O_RDONLY, 0);
    if(pfd == -1) lastWords(efd, "Open task monitor failed\n"sv);

    cline::USock usock; cline::Timer timer; cline::Worker worker;
    /* ===================  Set up events and poll  =================== */
    std::array<pollfd, idCnt> pollfds = {};

    const int tfd = timer.init();
    if(tfd == -1) lastWords(efd, "Create timerfd failed\n"sv);
    pollfds[tim].fd = tfd;
    pollfds[tim].events = POLLIN;

    const int sfd = usock.bind(getenv("CLINE_SOCK"));
    if(sfd < 0) lastWords(efd, "Bind unix socket failed\n"sv);
    pollfds[msg].fd = sfd;
    pollfds[msg].events = POLLIN | POLLHUP;

    const int ofd = redirFD(STDOUT_FILENO); // some dup magic to capture stdout
    if(ofd < 0) lastWords(efd, "Set up stdout capture failed\n"sv);
    pollfds[out].fd = ofd;
    pollfds[out].events = POLLIN;

    if(signal(SIGPIPE, SIG_IGN) == SIG_ERR) ;

    /* ========================  Set up cling  ======================== */
    llvm::llvm_shutdown_obj shutdownTrigger;

    llvm::sys::PrintStackTraceOnErrorSignal(argv[0]);
    llvm::PrettyStackTraceProgram X(argc, argv);

    cling::Interpreter interpreter(argc, argv);
    if (!interpreter.isValid()) lastWords(efd, "Set up interpreter failed\n"sv);

    cline::Processor processor(usock, interpreter);

    /* ======================  Start event loop  ====================== */
    constexpr size_t buflen = 64 * 1024;
    unsigned int heartbeats = 1;
    char primaryBuf  [buflen], // for code to be interpreted
         secondaryBuf[buflen]; // for data received during processing

    usock.send(getMsgHdr(mHELLO));  // handshake

    while(true) try{
        int ready = poll(pollfds.data(), idCnt, idleTime);
        if (ready == -1) lastWords(efd, "Error when polling\n"sv);
            /* EINTR would never happen unless SA_RESTART is set
               for EFAULT, EINVAL, ENOMEM, there's nothing we can do */

        if(pollfds[tim].revents & POLLIN) {
            /* *  Interpreter no response after timeout or joined  * */
            clearFD(tfd);
            if(worker.isAlive()) { /* timeout */
                if(!worker.cancel()) {
                    usock.send(getMsgHdr(mFATAL), "Thread cancellation failed"sv);
                    lastWords(efd, "Thread cancellation failed\n"sv);
                }
                usock.send(getMsgHdr(mTIMEOUT));
            } else {               /* joined */
                usock.send(getMsgHdr(mJOINED));
            }

        } else if(pollfds[msg].revents & POLLIN) {
            /* * * * * * * *  New request from client  * * * * * * * */
            /* When processor is working, it owns the primary buffer */
            char* buf = worker.notJoined() ? secondaryBuf : primaryBuf;
            int len = read(sfd, buf, buflen);
            switch(buf[0]) {
              case rINTERPRET:
                if(len == buflen) {
                    usock.send(getMsgHdr(mREJECTED), "Invalid input: too long"sv);
                    break;
                }
                if(worker.notJoined() && worker.isAlive()) {
                    usock.send(getMsgHdr(mPROCESSING));
                } else {
                    if(countThreads(pfd) != 1) {
                        usock.send(getMsgHdr(mFATAL), "Untracked thread(s) detected"sv);
                        lastWords(efd, "Untracked thread(s) detected\n"sv);
                    }

                    worker.spawn(processor.process(std::string_view(buf + 1, len - 1)));
                    timer.set(blockTime);
                }
                break;

              case rSOFTRESET:
                if(worker.notJoined()) {
                    /* avoid flooding */
                    std::this_thread::sleep_for(std::chrono::milliseconds(300));
                    timer.reset();     // it can't fail so don't check the rc
                    if(!worker.cancel()) {
                        usock.send(getMsgHdr(mFATAL), "Thread cancellation failed"sv);
                        lastWords(efd, "Thread cancellation failed\n"sv);
                    }
                    usock.send(getMsgHdr(mKILLED));
                } else {
                    /* not in processing */
                }
                break;

              case rKEEPALIVE:
                /* Kill stale sessions & avoid flooding */
                if(constexpr unsigned int life = 60 /* m */; ++heartbeats > life) {
                    usock.send(getMsgHdr(mLONG));
                    goto eventLoopEnd;
                }
                usock.send(getMsgHdr(mOK));
                break;

              default:
                usock.send(getMsgHdr(mREJECTED), "Unknown request type"sv);
            }

        } else if(pollfds[msg].revents & POLLHUP) {
            /* * * * * * * * *  Client disconnected  * * * * * * * * */

            break;
            
        } else if(pollfds[out].revents & POLLIN) {
            /* * * * * * * * * *  Stdin loopback * * * * * * * * * * */
            /* Only when the code is interpreted it can print something
               to stdout, thus we don't care primaryBuf anymore now */
            int len = read(ofd, primaryBuf, buflen);
            if(len == buflen) {
                /* printed message too long, discard it and inform the clinet */
                usock.send(getMsgHdr(mREJECTED),
                           "Too much data written to stdout. Discarding..."sv);
                flushFD(ofd);
                continue;
            }

            usock.send(getMsgHdr(mSTDOUT), std::string_view(primaryBuf, len));
        } else {
            short err = 0;
            for(pollfd& p: pollfds) err |= p.revents;
            if(err & POLLERR) lastWords(efd, "poll received POLLERR\n"sv);

            /* poll timed out */
            usock.send(getMsgHdr(mWARNING), "Inactive session expired"sv);
            break;
        }

    } catch(cline::SySExcept& e) {
        lastWords(efd, "Syscall error\n"sv); // TODO: say something about the error
    } catch(std::exception& e) {
        lastWords(efd, "Caught exception\n"sv); // TODO: same as above
    }

    eventLoopEnd: ;
    exit(EXIT_SUCCESS);
}
