#include "cm_match_box.hpp"


using namespace cm;

CTemplateShapeBoxType::CTemplateShapeBoxType(float inScaleX, float inScaleY) : CTemplatePartGroup(inScaleX, inScaleY) {
    mTotalX = 0;
    mTotalY = 0;
}

AnswerType CTemplateShapeBoxType::GenerateTemplate(std::shared_ptr<Component> para_part) {
    AnswerType answer = IMG_SUCCESS_ANS().SetErrCode();

    /***   获取参数   ***/
    const float meanScale = (mScaleX + mScaleY) * 0.5;
    double mTotalX = para_part->GetCommonData().GetComponentLenth() / meanScale;
    double mTotalY = para_part->GetCommonData().GetComponentWidth() / meanScale;

    /***   绘制模板   ***/
    // 计算模板尺寸
    const int maxSize = ceil(max(mTotalX, mTotalY));
    mTplSize = cv::Size(maxSize + mBorWidth * 2, maxSize + mBorWidth * 2);
    mTplCtr = cv::Point2f((mTplSize.width - 1) * 0.5, (mTplSize.height - 1) * 0.5);

    HalconCpp::HImage Image;
    Image.GenImageConst("byte", mTplSize.width, mTplSize.height);

	// 创建矩形轮廓
    HalconCpp::HObject Rect;
	GenRectangle2ContourXld(&Rect, 0, 0, 0, mTotalX * 0.5, mTotalY * 0.5);

    // 创建Shape模型
	CreateShapeModelXld(Rect,
		mPyramidLevels,
		DegToRad(-30),
		DegToRad(60),
		"auto", "auto", "ignore_local_polarity",
		5, &mModelID);

#ifdef _DEBUG
    //VisualizeHObjectWin(Rect, mTplSize);
    HalconCpp::HObject h1;
    GetTranslatedShapeTemplate(Rect, h1, mTplCtr);
    VisualizeHObjectWin(h1, mTplSize);
#endif
    
    return answer;
}
