#include <cerrno>
#include <csignal>
#include <cstdlib>

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

#include "./ring2.h"
#include "./handler.h"

using namespace cline::protocol;
using namespace std::string_view_literals;

constexpr time_t blockTime = 10 /* s */;
constexpr int idleTime = 1000*60*10 /*10min*/;
enum Fds {ftim = 0, fmsg, fout, ferr, idCnt};

int main(int argc, char *argv[]) {
    if(prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0) != 0) /* unreachable */;

    cline::Ring2 lineRec(4);
    lineRec.reserve(4096);

    const int efd = dup(STDERR_FILENO);
    if(efd == -1) /* unlikely, but let's check */ exit(EXIT_FAILURE);

    const auto lastWords = [&, efd](std::string_view msg) {
        char buf[16];
        int size = snprintf(buf, sizeof(buf), "cline[%d]: ", getpid());
        if(size > 0) axio::write(efd, {
            std::string_view(buf, size), msg, "last processed lines:\n"sv,
            lineRec.tail(), lineRec.head()
        });
        exit(EXIT_FAILURE);
    };

    const int pfd = open(getenv("CLINE_TASKS"), O_RDONLY, 0);
    if(pfd == -1) lastWords("Open tasks monitor failed\n"sv);

    cline::Worker worker;

    /* ========================  Set up cling  ======================== */
    llvm::llvm_shutdown_obj shutdownTrigger;

    llvm::sys::PrintStackTraceOnErrorSignal(argv[0]);
    llvm::PrettyStackTraceProgram X(argc, argv);

    cling::Interpreter interpreter(argc, argv);
    if (!interpreter.isValid()) lastWords("Set up interpreter failed\n"sv);

    /* ======================  Start event loop  ====================== */
    try {
        cline::Handler(blockTime, pfd, lineRec, interpreter)
        .startsvc(getenv("CLINE_SOCK"), idleTime);
    } catch(axio::InitError<axio::Timer>& e) {
        lastWords("Failed to create timerfd \n"sv);
    } catch(axio::InitError<axio::Socket>& e) {
        lastWords("Failed to bind unix socket \n"sv);
    } catch(axio::InitError<axio::ChanRX>& e) {
        lastWords("Failed to create pipe \n"sv);
    } catch(cline::svcError& e) {
        lastWords(e.what());
    } catch(cline::SySExcept& e) {
        lastWords("Syscall error\n"sv); // TODO: say something about the error
    } catch(std::exception& e) {
        lastWords("Caught exception\n"sv); // TODO: same as above
    }

    return 0;
}
