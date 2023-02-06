#include <string_view>

#include "./usock.h"

namespace cling {
    class Interpreter;
    class MetaProcessor;
}

namespace llvm {
    class raw_ostream;
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

class Processor {
  public:
    Processor(USock& usock, cling::Interpreter& interpreter);

    Processor* process(std::string_view buf) noexcept;

    void operator() ();

    void checkWrite(int res);

  private:
    static constexpr int timeout_ = 10 /* s */;
    std::string& intRes_;
    std::string& intErr_;
    std::string metaRes_;
    llvm::raw_string_ostream sstrm_;
    ClingCore clingCore_;
    std::string_view lo_;
    pthread_t* worker_;
    USock& usock_;

    void notifyErr(protocol::MsgType code, const char* msg);
};

} // namespace cline
