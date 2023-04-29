#include <functional>
#include <string_view>

#include "llvm/Support/raw_ostream.h"

#include "./protocol.h"

namespace cling {
    class Interpreter;
    class MetaProcessor;
}

namespace cline {

namespace protocol { enum MsgType: unsigned int; }

class ClingCore {
  public:
    ClingCore(cling::Interpreter& interp, llvm::raw_ostream& output);

    int interpret(std::string_view code);

    ~ClingCore(); // Prevent error when using incomplete type with unique_ptr

  private:
    std::unique_ptr<cling::MetaProcessor> metaProcessor_;
};

using protocol::MsgType;

class Processor {
  public:
    using Functor = std::function<void (MsgType, std::string_view)>;

    Processor(cling::Interpreter& interpreter, Functor onProcessed);

    Processor* process(std::string_view buf) noexcept;

    void operator() ();

  private:
    static constexpr int timeout_ = 10 /* s */;
    std::string& intRes_;
    std::string& intErr_;
    std::string metaRes_;
    llvm::raw_string_ostream sstrm_;
    ClingCore clingCore_;
    Functor onProcessed_;

    std::string_view lo_;
    pthread_t* worker_;

    void notifyErr(MsgType code, const char* msg);
};

} // namespace cline
