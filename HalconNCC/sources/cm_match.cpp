#include <ppl.h>
#include "cm_match.hpp"
#include "cm_svg.hpp"


using namespace cm;

AnswerType CTemplatePartGroup::TemplateMatch(const HalconCpp::HObject& inSrcImg, cv::Point2f& outOffset, float& outAngle, float& outScore) {
    AnswerType answer = IMG_SUCCESS_ANS().SetErrCode();

    HalconCpp::HTuple row, col, ang, score, width, height;
    HalconCpp::HTuple;
    GetImageSize(inSrcImg, &width, &height);
#ifdef _DEBUG
    cv::Mat showImg = HObjectToMat(inSrcImg);
#endif
    // false true
    std::string method = "true";
    FindNccModel(inSrcImg, mModelID, DegToRad(mBeginAngle), DegToRad(mEndAngle - mBeginAngle),
        0.8, 1, 0.5, method.c_str(), 0, &row, &col, &ang, &score);

    std::vector<double> hwidths = cm::tupleToVector<double>(width);
    std::vector<double> hheights = cm::tupleToVector<double>(height);
    std::vector<double> hrows = cm::tupleToVector<double>(row);
    std::vector<double> hcols = cm::tupleToVector<double>(col);
    std::vector<double> hangles = cm::tupleToVector<double>(ang);
    std::vector<double> hscores = cm::tupleToVector<double>(score);

    if (hscores.size() == 0) {
        FindNccModel(inSrcImg, mModelID, DegToRad(mBeginAngle), DegToRad(mEndAngle - mBeginAngle),
            0.3, 1, 0.5, method.c_str(), 0, &row, &col, &ang, &score);
        hwidths = cm::tupleToVector<double>(width);
        hheights = cm::tupleToVector<double>(height);
        hrows = cm::tupleToVector<double>(row);
        hcols = cm::tupleToVector<double>(col);
        hangles = cm::tupleToVector<double>(ang);
        hscores = cm::tupleToVector<double>(score);

        if (hscores.size() == 0) {
            hwidths = { 0 };
            hheights = { 0 };
            hrows = { 0 };
            hcols = { 0 };
            hangles = { 0 };
            hscores = { 0 };
        }
    }

    cv::Point2f imgCtr((hwidths[0] - 1) * 0.5, (hheights[0] - 1) * 0.5);
    outOffset = cv::Point2f(hcols[0], hrows[0]) - imgCtr;
    outAngle = -hangles[0] / CV_PI * 180;
    outScore = 50 + hscores[0] * 50;

    return answer;
}

AnswerType CTemplatePartGroup::GetTranslatedShapeTemplate(const HalconCpp::HObject& inSrcTpl, HalconCpp::HObject& outRotTpl, const cv::Point2f& inOffset) {
    AnswerType answer = IMG_SUCCESS_ANS().SetErrCode();

    HalconCpp::HTuple hv_ObjClass;
    HalconCpp::GetObjClass(inSrcTpl, &hv_ObjClass);
    std::string objClass = hv_ObjClass[0].S();

    HalconCpp::HTuple hHomMat2D;
    HalconCpp::HomMat2dIdentity(&hHomMat2D);

    double deltaRow = static_cast<double>(inOffset.y);
    double deltaCol = static_cast<double>(inOffset.x);

    HalconCpp::HomMat2dTranslate(hHomMat2D, deltaRow, deltaCol, &hHomMat2D);

    if (objClass == "region") {
        HalconCpp::AffineTransRegion(inSrcTpl, &outRotTpl, hHomMat2D, "nearest_neighbor");
    }
    else if (objClass == "xld_cont") {
        HalconCpp::AffineTransContourXld(inSrcTpl, &outRotTpl, hHomMat2D);
    }
    else if (objClass == "image") {
        HalconCpp::AffineTransImage(inSrcTpl, &outRotTpl, hHomMat2D, "constant", "false");
    }
    else {
        HalconCpp::AffineTransRegion(inSrcTpl, &outRotTpl, hHomMat2D, "nearest_neighbor");
    }

    return answer;
}

AnswerType CTemplatePartGroup::GetRotatedShapeTemplate(const HalconCpp::HObject& inSrcTpl, HalconCpp::HObject& outRotTpl, const cv::Point2f& inCtr, const float& inAng) {
    AnswerType answer = IMG_SUCCESS_ANS().SetErrCode();

    HalconCpp::HTuple hv_ObjClass;
    HalconCpp::GetObjClass(inSrcTpl, &hv_ObjClass);
    std::string objClass = hv_ObjClass[0].S();

    double center_row = inCtr.y;
    double center_col = inCtr.x;

    double radAng = static_cast<double>(-inAng) * CV_PI / 180.0;

    HalconCpp::HTuple hv_HomMat2D;
    HalconCpp::HomMat2dIdentity(&hv_HomMat2D);

    HalconCpp::HomMat2dRotate(hv_HomMat2D, radAng, center_row, center_col, &hv_HomMat2D);

    if (objClass == "region") {
        HalconCpp::AffineTransRegion(inSrcTpl, &outRotTpl, hv_HomMat2D, "nearest_neighbor");
    }
    else if (objClass == "xld_cont") {
        HalconCpp::AffineTransContourXld(inSrcTpl, &outRotTpl, hv_HomMat2D);
    }
    else if (objClass == "image") {
        HalconCpp::AffineTransImage(inSrcTpl, &outRotTpl, hv_HomMat2D, "constant", "false");
    }
    else {
        HalconCpp::AffineTransRegion(inSrcTpl, &outRotTpl, hv_HomMat2D, "nearest_neighbor");
    }

    return answer;
}

AnswerType CTemplatePartGroup::GetMergedShapeTemplate(const std::vector<HalconCpp::HObject>& inSrcTpls, HalconCpp::HObject& outMerTpl) {
    AnswerType answer = IMG_SUCCESS_ANS().SetErrCode();

    outMerTpl = inSrcTpls[0];
    if (inSrcTpls.size() > 1) {
        for (size_t i = 1; i < inSrcTpls.size(); i++) {
            HalconCpp::HObject temp;
            ConcatObj(outMerTpl, inSrcTpls[i], &temp);
            outMerTpl = temp;
        }
    }

    return answer;
}
