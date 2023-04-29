#include <cxxabi.h>

#include <pthread.h>
#include <sys/timerfd.h>
#include <unistd.h>

#include "cling/Interpreter/Exception.h"
#include "cling/MetaProcessor/MetaProcessor.h"
#include "cling/Utils/Output.h"

#include "llvm/Support/ErrorHandling.h"

#include "./processor.h"

using namespace cline::protocol;

namespace cline {

ClingCore::ClingCore(cling::Interpreter& interp, llvm::raw_ostream& output) {
    metaProcessor_.reset(new cling::MetaProcessor(interp, output));
    llvm::install_fatal_error_handler(
        &cling::CompilationException::throwingHandler
    );
}

ClingCore::~ClingCore() = default;

int ClingCore::interpret(std::string_view code) {
    cling::Interpreter::CompilationResult compRes;
    return metaProcessor_->process(llvm::StringRef(code), compRes);
}

Processor::Processor(cling::Interpreter& core, Functor onProcessed):
    intRes_(static_cast<llvm::raw_string_ostream&>(cling::outs()).str()),
    intErr_(static_cast<llvm::raw_string_ostream&>(cling::errs()).str()),
    metaRes_(), sstrm_(metaRes_), clingCore_(core, sstrm_),
    onProcessed_(onProcessed) {
        sstrm_.SetUnbuffered();
}

Processor* Processor::process(std::string_view buf) noexcept {
    lo_ = buf;
    return this;
}

void Processor::operator() () {
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

    int indent = 0;
    try{
        indent = clingCore_.interpret(lo_);
    } catch(const cling::InterpreterException& e) {
        return notifyErr(mINTERNAL, e.what());
    } catch (const std::bad_alloc& e) {
        /* posible oom, just terminate to be safe */
        std::string_view errMsg("Caught std::bad_alloc, terminating...");
        onProcessed_(mFATAL, errMsg);
        return;
    } catch(const std::exception& e) {
        return notifyErr(mEXCEPTION, e.what());
    } catch(const abi::__forced_unwind&) {     // throw by pthread_exit()
        /* If not caught here, it will fall below and nuke the process */
        throw;
    } catch(...) {
        /* Perhaps we should not catch ... - but let's do this for now */
        return notifyErr(mUNKNOWN, " ");
    }

    if(!metaRes_.empty()) {
        // TODO: send mLONG if message is too long (&below)
        onProcessed_(mINFO, metaRes_);
        metaRes_.clear();
        return;
    }

    if(!intErr_.empty()) {
        onProcessed_(mGRAMMAR, intErr_);
        intErr_.clear();
        intRes_.clear();
        return;
    }

    if(!intRes_.empty()) {
        onProcessed_(mDATA, intRes_);
        intRes_.clear();
        return;
    }

    if(indent > 0) {
        std::string len = std::to_string(indent);
        onProcessed_(mCONTINUE, len);
        return;
    }

    onProcessed_(mACCEPED, "");
}

void Processor::notifyErr(MsgType code, const char* msg) {
    metaRes_.clear(); intRes_.clear(); intErr_.clear();
    onProcessed_(code, std::string_view(msg));
}

} // namespace cline
