#include "cm_match_box.hpp"


using namespace cm;

CTemplateShapeBoxType::CTemplateShapeBoxType(float inScaleX, float inScaleY) : CTemplatePartGroup(inScaleX, inScaleY) {
    mTotalX = 0;
    mTotalY = 0;
}

AnswerType CTemplateShapeBoxType::GenerateTemplate(std::shared_ptr<Component> para_part) {
    AnswerType answer = IMG_SUCCESS_ANS().SetErrCode();

    /***   Get parameters   ***/
    const float meanScale = (mScaleX + mScaleY) * 0.5;
    double mTotalX = para_part->GetCommonData().GetComponentLenth() / meanScale;
    double mTotalY = para_part->GetCommonData().GetComponentWidth() / meanScale;

    /***   Draw template   ***/
    // Compute template size
    const int maxSize = ceil(max(mTotalX, mTotalY));
    mTplSize = cv::Size(maxSize + mBorWidth * 2, maxSize + mBorWidth * 2);
    mTplCtr = cv::Point2f((mTplSize.width - 1) * 0.5, (mTplSize.height - 1) * 0.5);

    HalconCpp::HImage Image;
    Image.GenImageConst("byte", mTplSize.width, mTplSize.height);

	// Create rectangle contour
    HalconCpp::HObject Rect;
	GenRectangle2ContourXld(&Rect, 0, 0, 0, mTotalX * 0.5, mTotalY * 0.5);

    // Create Shape model
	CreateShapeModelXld(Rect,
		mPyramidLevels,
        DegToRad(mBeginAngle), 
        DegToRad(mEndAngle - mBeginAngle),
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
