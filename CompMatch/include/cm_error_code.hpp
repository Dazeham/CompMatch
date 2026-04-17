#pragma once
#ifndef CM_ERROR_CODE_HPP
#define CM_ERROR_CODE_HPP


enum class ALGErrCode {
    IMG_CRASH = 1,
    IMG_PARAM_ERROR = 2,
    IMG_MATCH_FAIL = 3,
    IMG_INSPECT_FAIL = 4,
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
// 2
class IMG_PARAM_ERROR_ANS : public Answer {
public:
    IMG_PARAM_ERROR_ANS() { ANS = ALGErrCode::IMG_PARAM_ERROR; }
    enum class ANS2 {
        ImageTypeError,
        TemplateNumberError,
        ConvexHullPointsNumberError,
    };
    AnswerType SetErrCode(ANS2 inANS2) {
        return std::make_pair(ANS, (unsigned int)inANS2);
    }
};
// 3
class IMG_MATCH_FAIL_ANS : public Answer {
public:
    IMG_MATCH_FAIL_ANS() { ANS = ALGErrCode::IMG_MATCH_FAIL; }
    enum class ANS2 {
        CandidatePointsEmpty,
        LineFitFail,
        BoxPreciseModuleFail,
    };
    AnswerType SetErrCode(ANS2 inANS2) {
        return std::make_pair(ANS, (unsigned int)inANS2);
    }
};
// 4
class IMG_INSPECT_FAIL_ANS : public Answer {
public:
    IMG_INSPECT_FAIL_ANS() { ANS = ALGErrCode::IMG_INSPECT_FAIL; }
    enum class ANS2 {
        ElectrodeBrightnessAbnormal,
        ElectrodeAreaAbnormal,
        ElectrodeNumberError,
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
