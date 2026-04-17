#include "cm_match_box.hpp"


using namespace cm;

CTemplateShapeBoxType::CTemplateShapeBoxType(float inScaleX, float inScaleY) : CTemplatePartGroup(inScaleX, inScaleY) {
    mTotalX = 0;
    mTotalY = 0;
}

AnswerType CTemplateShapeBoxType::GenerateTemplate(std::shared_ptr<Component> para_part) {
    AnswerType answer = IMG_SUCCESS_ANS().SetErrCode();

    const float meanScale = (mScaleX + mScaleY) * 0.5;
    double mTotalX = para_part->GetCommonData().GetComponentLenth() / meanScale;
    double mTotalY = para_part->GetCommonData().GetComponentWidth() / meanScale;

    const int maxSize = ceil(max(mTotalX, mTotalY));
    mTplSize = cv::Size(maxSize + mBorWidth * 2, maxSize + mBorWidth * 2);
    mTplCtr = cv::Point2f((mTplSize.width - 1) * 0.5, (mTplSize.height - 1) * 0.5);

    HalconCpp::HImage Image;
    Image.GenImageConst("byte", mTplSize.width, mTplSize.height);

    HalconCpp::HRegion Rect;
    Rect.GenRectangle2(mTplCtr.y, mTplCtr.x, 0.0, mTotalX * 0.5, mTotalY * 0.5);

    HalconCpp::HObject ImgBg;
    GenImageConst(&ImgBg, "byte", mTplSize.width, mTplSize.height);

    HalconCpp::HObject ImgBgFilled;
    PaintRegion(ImgBg, ImgBg, &ImgBgFilled, 0, "fill");

    HalconCpp::HObject TemplateImg;
    PaintRegion(Rect, ImgBgFilled, &TemplateImg, 255, "fill");

    CreateNccModel(TemplateImg, mPyramidLevels, DegToRad(-30), DegToRad(60),
        "auto", "use_polarity", &mModelID);

#ifdef _DEBUG
    cv::Mat showImg = HObjectToMat(TemplateImg);
#endif

    return answer;
}
