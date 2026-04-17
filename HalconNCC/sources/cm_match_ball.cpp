#include <numeric>
#include "cm_match_ball.hpp"


using namespace cm;

CTemplateShapeBall::CTemplateShapeBall(float inScaleX, float inScaleY) : CTemplatePartGroup(inScaleX, inScaleY) {
	mTotalX = 0;
	mTotalY = 0;
	mMoldX = 0;
	mMoldY = 0;
	mNumX = 0;
	mNumY = 0;
	mAvgR = 0;
	mPitchX = 0;
	mPitchY = 0;
};

AnswerType CTemplateShapeBall::GenerateTemplate(std::shared_ptr<Component> inCompPtr) {
    AnswerType answer = IMG_SUCCESS_ANS().SetErrCode();

	const float meanScale = (mScaleX + mScaleY) * 0.5;
	mTotalX = inCompPtr->GetCommonData().GetComponentLenth() / meanScale;
	mTotalY = inCompPtr->GetCommonData().GetComponentWidth() / meanScale;

	std::shared_ptr<AbstractConfigurationBasic> pAbsMold = std::dynamic_pointer_cast<AbstractConfigurationBasic>(inCompPtr->GetOneAbstractConfigurationBasic(ConfigurationBasicType::MOLD));
	
	std::shared_ptr<MoldBasic> pMold = std::dynamic_pointer_cast<MoldBasic>(pAbsMold);
	MoldParam::ShapeParam moldShape = pMold->GetParam().GetShapeParam();
	mMoldX = moldShape.length / meanScale;
	mMoldY = moldShape.width / meanScale;

	std::shared_ptr<AbstractConfigurationBasic> pAbsBall = std::dynamic_pointer_cast<AbstractConfigurationBasic>(inCompPtr->GetOneAbstractConfigurationBasic(ConfigurationBasicType::BALL_GROUP));
	
	std::shared_ptr<BallGroupBasic> pBall = std::dynamic_pointer_cast<BallGroupBasic>(pAbsBall);
	BallGroupParam::ShapeParam ballShape = pBall->GetParam(0).GetShapeParam();
	std::vector<LackBallParam::LackParam> ballLack = pBall->GetParam(0).GetLackBallParam().GetAllLackParam();
	mNumX = ballShape.number_x;
	mNumY = ballShape.number_y;
	mAvgR = ballShape.diameter * 0.5 / meanScale;
	mPitchX = ballShape.pitch_x / meanScale;
	mPitchY = ballShape.pitch_y / meanScale;
	mLack = cv::Mat::ones(cv::Size(mNumX, mNumY), CV_8UC1);
	for (int i = 0; i < ballLack.size(); ++i) {
		LackBallParam::LackParam lack = ballLack[i];
		mLack(cv::Rect(lack.start_col - 1, lack.start_row - 1, lack.last_col - lack.start_col + 1, lack.last_row - lack.start_row + 1)).setTo(0);
	}

	const int maxSize = ceil(max((mNumX - 1) * mPitchX + 2 * mAvgR, (mNumY - 1) * mPitchY + 2 * mAvgR));
	mTplSize = cv::Size(maxSize + mBorWidth, maxSize + mBorWidth);
	mTplCtr = cv::Point2f((mTplSize.width - 1) * 0.5, (mTplSize.height - 1) * 0.5);

	HalconCpp::HImage Image;
	Image.GenImageConst("byte", mTplSize.width, mTplSize.height);

	HalconCpp::HObject ballSrcTpl;
	GetBallSourceTemplate(ballSrcTpl, mAvgR);

	HalconCpp::HObject ImgBg;
	GenImageConst(&ImgBg, "byte", mTplSize.width, mTplSize.height);

	HalconCpp::HObject ImgBgFilled;
	PaintRegion(ImgBg, ImgBg, &ImgBgFilled, 0, "fill");

	HalconCpp::HObject TemplateImg;
	PaintRegion(ballSrcTpl, ImgBgFilled, &TemplateImg, 255, "fill");

	CreateNccModel(TemplateImg, mPyramidLevels, DegToRad(-30), DegToRad(60),
		"auto", "use_polarity", &mModelID);

#ifdef _DEBUG
	cv::Mat showImg = HObjectToMat(TemplateImg);
#endif

    return answer;
}

AnswerType CTemplateShapeBall::GetBallTemplate(HalconCpp::HObject& outLeadTpl, const float& inRadius) {
	AnswerType answer = IMG_SUCCESS_ANS().SetErrCode();

	HalconCpp::HRegion hBall;
	hBall.GenCircle(mTplCtr.y, mTplCtr.x, inRadius);

#ifdef _DEBUG
	cv::Mat showImg = VisualizeHObject(hBall, mTplSize);
#endif

	outLeadTpl = hBall;

	return answer;
}

AnswerType CTemplateShapeBall::GetBallSourceTemplate(HalconCpp::HObject& outBallCompTpl, const float& inRadius) {
	AnswerType answer = IMG_SUCCESS_ANS().SetErrCode();

	HalconCpp::HObject ballTemplate;
	GetBallTemplate(ballTemplate, inRadius);

	cv::Mat tmpLack = mLack.clone();

	const cv::Point2f lackCenter((tmpLack.cols - 1) * 0.5, (tmpLack.rows - 1) * 0.5);
	std::vector<HalconCpp::HObject> ballTemplates;
	for (int rowNo = 0; rowNo < tmpLack.rows; ++rowNo) {
		const uchar* pRow = tmpLack.ptr<uchar>(rowNo);
		for (int colNo = 0; colNo < tmpLack.cols; ++colNo) {
			if (pRow[colNo] != 0) {
				HalconCpp::HObject tmpBallballTemplate;
				GetTranslatedShapeTemplate(ballTemplate, tmpBallballTemplate, cv::Point2f((colNo - lackCenter.x) * mPitchX, (rowNo - lackCenter.y) * mPitchY));
				ballTemplates.push_back(tmpBallballTemplate);
			}
		}
	}

	GetMergedShapeTemplate(ballTemplates, outBallCompTpl);

#ifdef _DEBUG
	cv::Mat showImg = VisualizeHObject(outBallCompTpl, mTplSize);
#endif

	return answer;
}
