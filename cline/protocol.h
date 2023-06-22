#pragma once

#include <array>
#include <string_view>

namespace cline::protocol {

enum ReqType: char {
    rINTERPRET = '0',
    rEMULINPUT,
    rSOFTRESET,
    rKEEPALIVE = '9'
};

enum MsgType: unsigned int {
    mACCEPED = 0,
    mBANNED,
    mCONTINUE,
    mDATA,
    mEXCEPTION,
    mFATAL,
    mGRAMMAR,
    mHELLO,
    mINFO,
    mJOINED,
    mKILLED,
    mLONG,
    mMETA,
    mINTERNAL,
    mOK,
    mPROCESSING,
    mUNUSED2,
    mREJECTED,
    mSLEEP,
    mTIMEOUT,
    mUNKNOWN,
    mUNUSED3,
    mWARNING,
    mSTDOUT,
    mSTDERR,
    mUNUSED4
};

inline std::string_view getMsgHdr(MsgType type) {
    static const std::array<char, 26> headers {
        'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z'
    };

    return std::string_view(&headers[type], 1);
}

} // namespace cline
