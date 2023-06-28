#include <stdexcept>

#include "channel.h"
#include "timer.h"
#include "socket.h"
#include "bufReader.h"
#include "ExtDispatcher.h"

#include "./processor.h"
#include "./ring2.h"
#include "./worker.h"

namespace cline {

class svcError: public std::runtime_error {
  public:
    using std::runtime_error::runtime_error;
};

class Handler {
  public:
    Handler(int blockTime, int taskChk, Ring2& rec, cling::Interpreter& it);

    void startsvc(const char* unixaddr, int maxidle);


  private:
    bool inactive();

    bool timeout(axio::Emitter emitter);
    bool socketi(axio::Emitter emitter);
    bool socketx(axio::Emitter emitter);
    bool stdouti(axio::Emitter emitter);
    bool stderri(axio::Emitter emitter);
    bool pollerr(axio::Emitter emitter);

    void respond(MsgType hdr);
    void respond(MsgType hdr, std::string_view body);

    std::string_view read(axio::Emitter emitter);

    void checkThreadsLimit(size_t limit);

    enum Fds {ftim = 0, fmsg, fout, ferr, fdCnt};
    static constexpr size_t buflen = 64 * 1024;
    using Buf = axio::bufReader<buflen>;

    static int emulateInput(std::string_view msg);

    const int blockTime_;

    unsigned int heartbeats = 1;

    int    taskChk_;
    Ring2& lineRec_;

    Worker worker;
    Buf primaryBuf, secondaryBuf;

    pollfd fds[fdCnt];
    axio::ExtDispatcher dispatcher;
    axio::Timer  timer;
    axio::Socket usock;

    Processor   processor_;
};

}
