#include <numeric>
#include "cm_match_ball.hpp"
#include "cm_affine.hpp"
#include "cm_intrinsics.hpp"


using namespace cm;

CTemplateShapeBall::CTemplateShapeBall(float inScaleX, float inScaleY) : CTemplatePartGroup(inScaleX, inScaleY) {
	// 元件参数
	mTotalX = 0;
	mTotalY = 0;
	mMoldX = 0;
	mMoldY = 0;
	mNumX = 0;
	mNumY = 0;
	mAvgR = 0;
	mPitchX = 0;
	mPitchY = 0;

	// 匹配参数
	mStepNum = 0;
	mAngleRange = 0;
};

AnswerType CTemplateShapeBall::GenerateTemplate(std::shared_ptr<Component> inCompPtr) {
	AnswerType answer = IMG_SUCCESS_ANS().SetErrCode();

	// 获取刻度
	const double minScale = std::min(mScaleX, mScaleY);
	mScaleFactor = cv::Point2d(mScaleX < mScaleY ? 1 : minScale / mScaleX, mScaleY < mScaleX ? 1 : minScale / mScaleY);

	// 获取球珊元件参数
	mTotalX = inCompPtr->GetCommonData().GetComponentLenth() / minScale;
	mTotalY = inCompPtr->GetCommonData().GetComponentWidth() / minScale;

	std::shared_ptr<AbstractConfigurationBasic> pAbsMold = std::dynamic_pointer_cast<AbstractConfigurationBasic>(inCompPtr->GetOneAbstractConfigurationBasic(ConfigurationBasicType::MOLD));
	
	std::shared_ptr<MoldBasic> pMold = std::dynamic_pointer_cast<MoldBasic>(pAbsMold);
	MoldParam::ShapeParam moldShape = pMold->GetParam().GetShapeParam();
	mMoldX = moldShape.length / minScale;
	mMoldY = moldShape.width / minScale;

	std::shared_ptr<AbstractConfigurationBasic> pAbsBall = std::dynamic_pointer_cast<AbstractConfigurationBasic>(inCompPtr->GetOneAbstractConfigurationBasic(ConfigurationBasicType::BALL_GROUP));
	
	std::shared_ptr<BallGroupBasic> pBall = std::dynamic_pointer_cast<BallGroupBasic>(pAbsBall);
	BallGroupParam::ShapeParam ballShape = pBall->GetParam(0).GetShapeParam();
	std::vector<LackBallParam::LackParam> ballLack = pBall->GetParam(0).GetLackBallParam().GetAllLackParam();
	mNumX = ballShape.number_x;
	mNumY = ballShape.number_y;
	mAvgR = ballShape.diameter * 0.5 / minScale;
	mPitchX = ballShape.pitch_x / minScale;
	mPitchY = ballShape.pitch_y / minScale;
	mLack = cv::Mat::ones(cv::Size(mNumX, mNumY), CV_8UC1);
	for (int i = 0; i < ballLack.size(); ++i) {
		LackBallParam::LackParam lack = ballLack[i];
		mLack(cv::Rect(lack.start_col - 1, lack.start_row - 1, lack.last_col - lack.start_col + 1, lack.last_row - lack.start_row + 1)).setTo(0);
	}

	// 绘制源模板 无缩放 无旋转 模板超限保护
	std::vector<cv::Mat> ballSrcTpl;
	answer = GetBallSourceTemplate(ballSrcTpl, mAvgR, 1, 0);
	if (answer.first != ALGErrCode::IMG_SUCCESS) return answer;
	
	float baseSplStep = 1;
	GetBaseSampleStep(ballSrcTpl, baseSplStep);

	// 设置匹配参数
	mStepNum = 3;
	//if ((mTotalX + mTotalY) >= 60) {
		mPyramidLevels = { 2, 1, 0 };
		mStepPixels = { 1, 1, 1 };
		mSobelSizes = { 3, 3, 3 };
	//}
	//else {
	//	mPyramidLevels = std::vector<int>{ 1, 1, 0 };
	//	mStepPixels = { 2, 1, 1 };
	//	mSobelSizes = { 3, 3, 3 };
	//}
	//mStepAngles = { 3, 0.5, 0.1 };
	GetStepAngles(cv::Size2d(mTotalX, mTotalY), mPyramidLevels, mStepAngles);
	mAngleRange = mStepAngles[0];
	
	// 绘制球珊元件正模板
	mSampleSteps = { baseSplStep * mSplItv, baseSplStep * mSplItv, baseSplStep * mSplItv };
	mBallStepTemplates.resize(mStepNum);
	for (int stepNo = 0; stepNo < mStepNum; ++stepNo) {
		GetBallSourceTemplate(mBallStepTemplates[stepNo], mAvgR, mSampleSteps[stepNo], mPyramidLevels[stepNo]);
	}

	// 严格限制模板规模
	GetStrictSampleShapeTemplates(mBallStepTemplates, mBallStepTemplates);

# ifdef _DEBUG
	for (int i = 0; i < mBallStepTemplates.size(); ++i) {
		CheckShapeTemplates(mBallStepTemplates[i]);
	}
# endif

	// 绘制旋转模板
	GetRotatedShapeTemplates(mBallStepTemplates[0], mBallRotTemplatesCoarse, mBeginAngle + mAngleRange, mEndAngle - mAngleRange, mStepAngles[0]);
	GetScaleShapeTemplates(mBallRotTemplatesCoarse, mScaleFactor);
	GetRotatedShapeTemplates(mBallStepTemplates[1], mBallRotTemplates, mBeginAngle, mEndAngle, mStepAngles[1]);
	GetScaleShapeTemplates(mBallRotTemplates, mScaleFactor);

	// 绘制多尺寸模板
	std::vector<float> vecRatio = { 1.0 };
	mBallMultiScaleTemplates.resize(vecRatio.size());
	for (int i = 0; i < vecRatio.size(); ++i) {
		float tmpRadius = mAvgR * vecRatio[i];
		GetBallSourceTemplate(mBallMultiScaleTemplates[i], tmpRadius, mSampleSteps[2], mPyramidLevels[2]);
	}

	// 绘制单个焊球模板用于球珊结果检查
	GetSingleBallSourceTemplate(mBallTemplates, mAvgR, 1, 0);

	return answer;
}

AnswerType CTemplateShapeBall::TemplateMatch(const cv::Mat& inSrcImg, cv::Point2f& outOffset, float& outAngle, float& outScore) {
	AnswerType answer = IMG_SUCCESS_ANS().SetErrCode();

	// 截图
	mPartImg = inSrcImg;
	cv::Size2f gbaSize(mPitchX * (mLack.cols - 1) + mAvgR * 2, mPitchY * (mLack.rows - 1) + mAvgR * 2);

	// 匹配
	cv::Point2f offset;
	float angle = 0.;
	float score = 0.;
	std::vector<cv::Mat> cropGradImgs;
	cv::Point leftTop;
	float maxMagVal;
	cv::Mat crsMagImg;
	answer = PartDetect(cv::Point2f(mScaleX, mScaleY), mPartImg, mBallStepTemplates, mBallRotTemplatesCoarse, mBallRotTemplates, mBallMultiScaleTemplates, mPyramidLevels, mStepPixels, mBeginAngle, mEndAngle, mAngleRange, mStepAngles, mSobelSizes, gbaSize, gbaSize, mStepNum, offset, angle, score, crsMagImg, mCropImg, cropGradImgs, mCropMagImg, maxMagVal, mLeftTop, mScaleFactor);
	if (answer.first != ALGErrCode::IMG_SUCCESS) return answer;

	// 箱形元件额外精度模块
#if showTimeFlag
	double time_start = cv::getTickCount();  // 计时开始
#endif
	mSrcImgCtr = cv::Point2f((mPartImg.size().width - 1) * 0.5, (mPartImg.size().height - 1) * 0.5);
	mCropImgCtr = cv::Point2f((mCropImg.cols - 1) * 0.5, (mCropImg.rows - 1) * 0.5);
	const cv::Point2f inScale(mScaleX, mScaleY);
	mSclFac = cv::Point2d(inScale.x < inScale.y ? 1 : inScale.x / inScale.y, inScale.y < inScale.x ? 1 : inScale.y / inScale.x);
	// 亚像素方法 开始
	/*
	answer = GetPrecisePosition(mSrcImgCtr, mScaleFactor, mBallMultiScaleTemplates[0], cv::Point2f(mScaleX, mScaleY), mPartImg.size(), mCropImg, mCropMagImg, cropGradImgs, mLeftTop, cv::Size2d(mTotalX, mTotalY), offset, angle);
	if (answer.first == ALGErrCode::IMG_SUCCESS) {
		std::vector<cv::Mat> tmpRotTpl;
		GetRotatedShapeTemplate(mBallMultiScaleTemplates[0], tmpRotTpl, angle);
		GetScaleShapeTemplate(tmpRotTpl, mScaleFactor);
		const cv::Point2f imgCenter((mPartImg.cols - 1) * 0.5, (mPartImg.rows - 1) * 0.5);
		cv::Point2f rOffset(offset.x * pow(0.5, mPyramidLevels[2]) - mLeftTop.x + imgCenter.x, offset.y * pow(0.5, mPyramidLevels[2]) - mLeftTop.y + imgCenter.y);
		score = cm::calculateGradientScore(cropGradImgs[0], cropGradImgs[1], tmpRotTpl[0], tmpRotTpl[1], tmpRotTpl[2], tmpRotTpl[3], rOffset.x, rOffset.y) * 100;
#ifdef _DEBUG
		CheckMatchTemplate(mPartImg, mBallMultiScaleTemplates[0], offset, angle, mPyramidLevels[2], 0, mScaleFactor);
		CheckMatchTemplateSVG(mPartImg, mBallMultiScaleTemplates[0], offset, angle, mPyramidLevels[2], 0, mScaleFactor);
#endif
	}
	else {
		answer = IMG_SUCCESS_ANS().SetErrCode();
	}
	*/
	// 亚像素方法 结束
#if showTimeFlag
	double tmpTime = (cv::getTickCount() - time_start) * 1000. / double(cv::getTickFrequency());
	std::cout << " - 精度补偿: " << tmpTime << "ms" << std::endl;  // 计时结束
#endif

	// 结果导出
	outOffset = offset;
	outAngle = angle;
	outScore = 50 + score * 0.5;

	return answer;
}

AnswerType CTemplateShapeBall::SaveResult(const cv::Mat& inSrcImg, const cv::Point2f& inOffset, const float& inAngle, const float& inScore, const AnswerType& inAnswer, const double& inTime, const std::string& inPath) {
	AnswerType answer = IMG_SUCCESS_ANS().SetErrCode();

	std::vector<cv::Mat> inSrcTpl = mBallMultiScaleTemplates[0];
	cv::Point2d inSclFac = mScaleFactor;
	int inPyrLvl = mPyramidLevels[2];
	int inMinPyrLvl = 0;

	int showImgSize = 512;
	int bodWidth = (inSrcImg.cols - showImgSize) * 0.5;
	//cv::Mat showImg = inSrcImg(cv::Rect(bodWidth, bodWidth, showImgSize, showImgSize)).clone();
	cv::Mat showImg = inSrcImg;
	//if (showImg.depth() != CV_32F) {
	//    showImg.convertTo(showImg, CV_32F);
	//}
	//if (showImg.channels() != 3) {
	//    cv::Mat c1 = showImg.clone();
	//    cv::Mat c2 = showImg.clone();
	//    cv::Mat c3 = showImg.clone();
	//    std::vector<cv::Mat> cs = { c1, c2, c3 };
	//    cv::merge(cs, showImg);
	//}

	std::vector<cv::Mat> rotTpl;
	GetRotatedShapeTemplate(inSrcTpl, rotTpl, inAngle);
	GetScaleShapeTemplate(rotTpl, inSclFac);

	cv::Point2f imgCenter((showImg.cols - 1) * 0.5, (showImg.rows - 1) * 0.5);
	cv::Point2f offset(imgCenter.x + inOffset.x * powf(0.5, inPyrLvl - inMinPyrLvl), imgCenter.y + inOffset.y * powf(0.5, inPyrLvl - inMinPyrLvl));

	//// 梯度模板
	//for (int i = 0; i < rotTpl[0].cols; i++) {
	//    int row = cvRound(rotTpl[1].ptr<float>()[i] + offset.y);
	//    int col = cvRound(rotTpl[0].ptr<float>()[i] + offset.x);
	//    if (row < 0 || row >= showImg.rows || col < 0 || col >= showImg.cols) {
	//        continue;
	//    }
	//    cv::Vec3f& pixel = showImg.at<cv::Vec3f>(row, col);
	//    pixel[0] = 255;
	//    pixel[1] = 0;
	//    pixel[2] = 0;
	//}

	//// 灰度模板
	//if (inSrcTpl.size() == 6) {
	//    for (int i = 0; i < rotTpl[4].cols; i++) {
	//        int row = cvRound(rotTpl[5].ptr<float>()[i] + offset.y);
	//        int col = cvRound(rotTpl[4].ptr<float>()[i] + offset.x);
	//        if (row < 0 || row >= showImg.rows || col < 0 || col >= showImg.cols) {
	//            continue;
	//        }
	//        cv::Vec3f& pixel = showImg.at<cv::Vec3f>(row, col);
	//        pixel[0] = 0;
	//        pixel[1] = 255;
	//        pixel[2] = 0;
	//    }
	//}

	// 保存SVG
	cv::Mat magImg;
	std::vector<cv::Mat> gradImgs;
	cm::getNormalizedGradientAndMagnitudeImages(showImg, gradImgs, magImg, true);
	//GetMaxBlurImage(magImg, magImg);

	const int roiWidth = 512;
	const int roiHeight = roiWidth;
	const int borWidth = (inSrcImg.cols - roiWidth) / 2;
	const int borHeight = (inSrcImg.rows - roiHeight) / 2;
	cv::Mat roiImg = inSrcImg(cv::Rect(borWidth, borHeight, roiWidth, roiHeight)).clone();
	cv::Point2f imgOfs((inSrcImg.cols - roiImg.cols) * 0.5, (inSrcImg.rows - roiImg.rows) * 0.5);
	SVGTool st(inPath, roiImg);

	// 显示点
	//st.drawCircle(cv::Point2f(0.5, 0.5), 0.5, "red");  // 原点
	//for (int i = 0; i < rotTpl[0].cols; i++) {  // 梯度模板
	//    float row = rotTpl[1].ptr<float>()[i] + offset.y;
	//    float col = rotTpl[0].ptr<float>()[i] + offset.x;
	//    if (row < 0 || row >= showImg.rows || col < 0 || col >= showImg.cols) {
	//        continue;
	//    }
	//    cv::Point2f centerPt(col, row);
	//    float radius = 0.6f;
	//    st.drawCircle(centerPt - imgOfs + cv::Point2f(0.5, 0.5), radius, "#C44DFF");
	//}
	//if (rotTpl.size() == 6) {  // 灰度模板
	//	if (rotTpl[4].cols > 0) {
	//		for (int i = 0; i < rotTpl[4].cols; i++) {
	//			float row = rotTpl[5].ptr<float>()[i] + offset.y;
	//			float col = rotTpl[4].ptr<float>()[i] + offset.x;
	//			if (row < 0 || row >= showImg.rows || col < 0 || col >= showImg.cols) {
	//				continue;
	//			}
	//			cv::Point2f centerPt(col, row);
	//			float radius = 0.6f;
	//			st.drawCircle(centerPt - imgOfs + cv::Point2f(0.5, 0.5), radius, "#FF8C00");
	//		}
	//	}
	//}

	// 显示轮廓线
	std::vector<SVGItem> vecBallSrcTpl;
	GetBallSourceItem(vecBallSrcTpl, mAvgR);
	GetRotatedItems(vecBallSrcTpl, vecBallSrcTpl, inAngle);
	GetTranslatedItems(vecBallSrcTpl, vecBallSrcTpl, offset - imgOfs + cv::Point2f(0.5, 0.5));
	for (int i = 0; i < vecBallSrcTpl.size(); ++i) {
		st.drawCircle(cv::Point2f(vecBallSrcTpl[i].cx, vecBallSrcTpl[i].cy), vecBallSrcTpl[i].w, "red", 2);
	}
	//st.drawRect(offset - imgOfs + cv::Point2f(0.5, 0.5), cv::Size2f(mTotalX, mTotalY), inAngle ,1, "red");

	float score = 0;
	if (inAnswer.first == ALGErrCode::IMG_SUCCESS) {
	    score = inScore;
	}
	else {
	    st.drawLine(cv::Point(1, 1), cv::Point(showImg.cols-1, showImg.rows-1), 2, "red");
	    st.drawLine(cv::Point(showImg.cols-1, 1), cv::Point(1, showImg.rows-1), 2, "red");
	}

	//// 显示分数
	//int fontSize = showImg.rows * 0.1;
	//st.drawText(cv::Point2f(1, fontSize), "red", std::to_string(fontSize) + "px", "Time: " + std::to_string(inTime) + " ms");
	//st.drawText(cv::Point2f(1, showImg.rows-1), "red", std::to_string(fontSize) + "px", "Score: " + std::to_string(score));

	st.close();

	return answer;
}

// SVG轮廓
AnswerType CTemplateShapeBall::GetBallItem(SVGItem& outLeadTpl, const float& inRadius) {
	AnswerType answer = IMG_SUCCESS_ANS().SetErrCode();

	outLeadTpl = SVGItem(0, 0, inRadius, inRadius, 0);

	return answer;
}

AnswerType CTemplateShapeBall::GetBallSourceItem(std::vector<SVGItem>& outBallCompTpl, const float& inRadius) {
	AnswerType answer = IMG_SUCCESS_ANS().SetErrCode();

	const float radius = inRadius;
	SVGItem ballTemplate;
	//if (2 * CV_PI * radius > 8) {
	//	GetBallTemplateFix(ballTemplate, radius, 8);
	//}
	//else {
	GetBallItem(ballTemplate, radius);
	//}

	cv::Mat tmpLack = mLack.clone();
	//if (std::min(tmpLack.cols, tmpLack.rows) >= 5) {
	//	std::vector<std::vector<cv::Point>> contours;
	//	cv::findContours(tmpLack, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE);
	//	cv::Mat conImg = cv::Mat::zeros(tmpLack.size(), CV_8UC1);
	//	cv::drawContours(conImg, contours, -1, cv::Scalar(1));
	//	tmpLack = conImg;
	//}

	const cv::Point2f lackCenter((tmpLack.cols - 1) * 0.5, (tmpLack.rows - 1) * 0.5);
	std::vector<SVGItem> ballTemplates;
	for (int rowNo = 0; rowNo < tmpLack.rows; ++rowNo) {
		const uchar* pRow = tmpLack.ptr<uchar>(rowNo);
		for (int colNo = 0; colNo < tmpLack.cols; ++colNo) {
			if (pRow[colNo] != 0) {
				SVGItem tmpBallballTemplate;
				GetTranslatedItem(ballTemplate, tmpBallballTemplate, cv::Point2f((colNo - lackCenter.x) * mPitchX, (rowNo - lackCenter.y) * mPitchY));
				ballTemplates.push_back(tmpBallballTemplate);
			}
		}
	}

	//GetMergedShapeTemplate(ballTemplates, outBallCompTpl);
	outBallCompTpl = ballTemplates;

	return answer;
}

// 模板绘制
AnswerType CTemplateShapeBall::GetBallTemplate(std::vector<cv::Mat>& outLeadTpl, const float& inRadius, const float& inSplStep) {
	AnswerType answer = IMG_SUCCESS_ANS().SetErrCode();

	const float radius = inRadius;
	const float perimeter = 2 * CV_PI * radius;
	const int ptNum = std::max(int(perimeter / inSplStep), 8);
	const float splStep = perimeter / ptNum;
	cv::Mat tX = cv::Mat::zeros(cv::Size(ptNum, 1), CV_32FC1);
	cv::Mat tY = cv::Mat::zeros(cv::Size(ptNum, 1), CV_32FC1);
	cv::Mat tGX = cv::Mat::zeros(cv::Size(ptNum, 1), CV_32FC1);
	cv::Mat tGY = cv::Mat::zeros(cv::Size(ptNum, 1), CV_32FC1);
	float* pX = tX.ptr<float>();
	float* pY = tY.ptr<float>();
	float* pGX = tGX.ptr<float>();
	float* pGY = tGY.ptr<float>();
	float angle = 0;
	for (int ptNo = 0; ptNo < ptNum; ++ptNo) {
		angle = ptNo * splStep / radius;
		pX[ptNo] = radius * cosf(angle);
		pY[ptNo] = radius * sin(angle);
		pGX[ptNo] = -cosf(angle);
		pGY[ptNo] = -sin(angle);
	}


	cv::Mat tVX, tVY;
	///*
	const float useRatio = 0.9;
	const float radiusV = useRatio * radius;
	const float perimeterV = 2 * CV_PI * radiusV;
	const int ptNumV = std::max(int(perimeterV / inSplStep), 8);
	const float splStepV = perimeterV / ptNumV;
	tVX = cv::Mat::zeros(cv::Size(ptNumV, 1), CV_32FC1);
	tVY = cv::Mat::zeros(cv::Size(ptNumV, 1), CV_32FC1);
	float* pVX = tVX.ptr<float>();
	float* pVY = tVY.ptr<float>();
	for (int ptNo = 0; ptNo < ptNumV; ++ptNo) {
		angle = ptNo * splStepV / radiusV;
		pVX[ptNo] = radiusV * cosf(angle);
		pVY[ptNo] = radiusV * sin(angle);
	}
	//*/

	if (tVX.rows > 0) {
		outLeadTpl = { tX, tY, tGX, tGY, tVX, tVY };
	}
	else {
		outLeadTpl = { tX, tY, tGX, tGY };
	}

	return answer;
}

AnswerType CTemplateShapeBall::GetBallTemplateFix(std::vector<cv::Mat>& outLeadTpl, const float& inRadius, const int& inSplNum) {
	AnswerType answer = IMG_SUCCESS_ANS().SetErrCode();

	const float perimeter = 2 * CV_PI * inRadius;
	const int ptNum = inSplNum;
	cv::Mat tX = cv::Mat::zeros(cv::Size(ptNum, 1), CV_32FC1);
	cv::Mat tY = cv::Mat::zeros(cv::Size(ptNum, 1), CV_32FC1);
	cv::Mat tGX = cv::Mat::zeros(cv::Size(ptNum, 1), CV_32FC1);
	cv::Mat tGY = cv::Mat::zeros(cv::Size(ptNum, 1), CV_32FC1);
	float* pX = tX.ptr<float>();
	float* pY = tY.ptr<float>();
	float* pGX = tGX.ptr<float>();
	float* pGY = tGY.ptr<float>();
	float angle = 0;
	for (int ptNo = 0; ptNo < ptNum; ++ptNo) {
		angle = ptNo * CV_PI * 2 / float(ptNum);
		pX[ptNo] = inRadius * cosf(angle);
		pY[ptNo] = inRadius * sin(angle);
		pGX[ptNo] = -cosf(angle);
		pGY[ptNo] = -sin(angle);
	}


	cv::Mat tVX, tVY;
	///*
	const float useRatio = 0.9;
	const float radiusV = useRatio * inRadius;
	tVX = cv::Mat::zeros(cv::Size(ptNum, 1), CV_32FC1);
	tVY = cv::Mat::zeros(cv::Size(ptNum, 1), CV_32FC1);
	float* pVX = tVX.ptr<float>();
	float* pVY = tVY.ptr<float>();
	for (int ptNo = 0; ptNo < ptNum; ++ptNo) {
		angle = ptNo * CV_PI * 2 / float(ptNum);
		pVX[ptNo] = radiusV * cosf(angle);
		pVY[ptNo] = radiusV * sin(angle);
	}
	//*/

	if (tVX.rows > 0) {
		outLeadTpl = { tX, tY, tGX, tGY, tVX, tVY };
	}
	else {
		outLeadTpl = { tX, tY, tGX, tGY };
	}

	return answer;
}

AnswerType CTemplateShapeBall::GetBallSourceTemplate(std::vector<cv::Mat>& outBallCompTpl, const float& inRadius, const float& inSplStep, const int& inPyrLvl) {
	AnswerType answer = IMG_SUCCESS_ANS().SetErrCode();

	const float pyrfactor = powf(0.5, inPyrLvl);
	const float radius = inRadius * pyrfactor;
	std::vector<cv::Mat> ballTemplate;
	//if (2 * CV_PI * radius > 8) {
	//	GetBallTemplateFix(ballTemplate, radius, 8);
	//}
	//else {
		GetBallTemplate(ballTemplate, radius, inSplStep);
	//}

	cv::Mat tmpLack = mLack.clone();
	if (std::min(tmpLack.cols, tmpLack.rows) >= 5) {
		std::vector<std::vector<cv::Point>> contours;
		cv::findContours(tmpLack, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE);
		cv::Mat conImg = cv::Mat::zeros(tmpLack.size(), CV_8UC1);
		cv::drawContours(conImg, contours, -1, cv::Scalar(1));
		tmpLack = conImg;
	}

	const cv::Point2f lackCenter((tmpLack.cols - 1) * 0.5, (tmpLack.rows - 1) * 0.5);
	std::vector<std::vector<cv::Mat>> ballTemplates;
	for (int rowNo = 0; rowNo < tmpLack.rows; ++rowNo) {
		const uchar* pRow = tmpLack.ptr<uchar>(rowNo);
		for (int colNo = 0; colNo < tmpLack.cols; ++colNo) {
			if (pRow[colNo] != 0) {
				std::vector<cv::Mat> tmpBallballTemplate;
				GetTranslatedShapeTemplate(ballTemplate, tmpBallballTemplate, cv::Point2f((colNo - lackCenter.x) * mPitchX * pyrfactor, (rowNo - lackCenter.y) * mPitchY * pyrfactor));
				ballTemplates.push_back(tmpBallballTemplate);
			}
		}
	}

	GetMergedShapeTemplate(ballTemplates, outBallCompTpl);

	return answer;
}

AnswerType CTemplateShapeBall::GetSingleBallSourceTemplate(std::vector<std::vector<cv::Mat>>& outBallCompTpl, const float& inRadius, const float& inSplStep, const int& inPyrLvl) {
	AnswerType answer = IMG_SUCCESS_ANS().SetErrCode();

	const float pyrfactor = powf(0.5, inPyrLvl);
	const float radius = inRadius * pyrfactor;
	std::vector<cv::Mat> ballTemplate;
	GetBallTemplate(ballTemplate, radius, inSplStep);

	cv::Mat tmpLack = mLack.clone();
	const cv::Point2f lackCenter((tmpLack.cols - 1) * 0.5, (tmpLack.rows - 1) * 0.5);
	std::vector<std::vector<cv::Mat>> ballTemplates;
	for (int rowNo = 0; rowNo < tmpLack.rows; ++rowNo) {
		const uchar* pRow = tmpLack.ptr<uchar>(rowNo);
		for (int colNo = 0; colNo < tmpLack.cols; ++colNo) {
			if (pRow[colNo] != 0) {
				std::vector<cv::Mat> tmpBallballTemplate;
				GetTranslatedShapeTemplate(ballTemplate, tmpBallballTemplate, cv::Point2f((colNo - lackCenter.x) * mPitchX * pyrfactor, (rowNo - lackCenter.y) * mPitchY * pyrfactor));
				ballTemplates.push_back(tmpBallballTemplate);
			}
		}
	}

	outBallCompTpl = ballTemplates;

	return answer;
}
