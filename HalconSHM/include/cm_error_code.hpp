#pragma once
#ifndef CM_ERROR_CODE_HPP
#define CM_ERROR_CODE_HPP


enum class ALGErrCode {
    IMG_CRASH = 1,
    IMG_SUCCESS = 50
};
typedef std::pair<ALGErrCode, unsigned int> AnswerType;

class Answer
{
public:
    ALGErrCode ANS;
};

// 1
class IMG_CRASH_ANS : public Answer {
public:
    IMG_CRASH_ANS() { ANS = ALGErrCode::IMG_CRASH; }
    enum class ANS2 {
        CPlusPlusException,
        OpenCVException,
        UnknowException
    };
    AnswerType SetErrCode(ANS2 inANS2) {
        return std::make_pair(ANS, (unsigned int)inANS2);
    }
};
// 50
class IMG_SUCCESS_ANS : public Answer {
public:
    IMG_SUCCESS_ANS() { ANS = ALGErrCode::IMG_SUCCESS; }
    enum class ANS2 {
        OK
    };
    AnswerType SetErrCode() {
        return std::make_pair(ANS, (unsigned int)ANS2::OK);
    }
};

#endif
