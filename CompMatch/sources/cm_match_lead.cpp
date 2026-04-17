#include "cm_match_lead.hpp"
#include "cm_intrinsics.hpp"
#include "cm_affine.hpp"
#include "cm_utils.hpp"
#include "cm_error_code.hpp"


using namespace cm;

CTemplateShapeLead::CTemplateShapeLead(float inScaleX, float inScaleY) : CTemplatePartGroup(inScaleX, inScaleY) {
	// 元件参数
	mTotalX = 0;
	mTotalY = 0;
	mMoldX = 0;
	mMoldY = 0;

	// 匹配参数
	mStepNum = 0;
	mAngleRange = 0;
}

AnswerType CTemplateShapeLead::GenerateTemplate(std::shared_ptr<Component> inCompPtr) {
	AnswerType answer = IMG_SUCCESS_ANS().SetErrCode();

	// 获取刻度
	const double minScale = std::min(mScaleX, mScaleY);
	mScaleFactor = cv::Point2d(mScaleX < mScaleY ? 1 : minScale / mScaleX, mScaleY < mScaleX ? 1 : minScale / mScaleY);

	// 获取引脚元件参数
	mTotalX = inCompPtr->GetCommonData().GetComponentLenth() / minScale;
	mTotalY = inCompPtr->GetCommonData().GetComponentWidth() / minScale;

	std::shared_ptr<AbstractConfigurationBasic> pAbsLead = std::dynamic_pointer_cast<AbstractConfigurationBasic>(inCompPtr->GetOneAbstractConfigurationBasic(ConfigurationBasicType::LEAD_GROUP));
	std::shared_ptr<AbstractConfigurationBasic> pAbsMold = std::dynamic_pointer_cast<AbstractConfigurationBasic>(inCompPtr->GetOneAbstractConfigurationBasic(ConfigurationBasicType::MOLD));

	std::shared_ptr<LeadGroupBasic> pLead = std::dynamic_pointer_cast<LeadGroupBasic>(pAbsLead);
	std::shared_ptr<MoldBasic> pMold = std::dynamic_pointer_cast<MoldBasic>(pAbsMold);
	MoldParam moldParam = pMold->GetParam();

	mMoldX = moldParam.GetShapeParam().length / minScale;
	mMoldY = moldParam.GetShapeParam().width / minScale;

	mVecLeadNum.resize(4);	           //引脚数目
	std::fill(mVecLeadNum.begin(), mVecLeadNum.end(), 0);
	mVecLeadPitch.resize(4);	           //引脚间距
	mVecLeadLength.resize(4);           //引脚长度
	mVecLeadWidth.resize(4);	           //引脚宽度
	mVecCenterX.resize(4);              //引脚组中心x
	mVecCenterY.resize(4);              //引脚组中心y
	mVecCutParam.resize(4);             // 缺失引脚

	for (int i = 0; i < pLead->ParamCount(); ++i) {
		LeadGroupParam leadParam = pLead->GetParam(i);
		auto it = mMapAngleToIdx.find(leadParam.GetShapeParam().angle);
		int idx = -1;
		if (it != mMapAngleToIdx.end()) {
			idx = it->second;
		}
		else {
			continue;
		}

		LeadGroupParam::ShapeParam leadShapeParam = leadParam.GetShapeParam();
		std::vector<CutLeadParam::ACutLeadParam> leadCutParam = leadParam.GetCutLeadParam().GetAllCutLeadParam();

		mVecLeadNum[idx] = leadShapeParam.number;
		mVecLeadPitch[idx] = leadShapeParam.pitch / minScale;
		mVecLeadLength[idx] = leadShapeParam.length / minScale;
		mVecLeadWidth[idx] = leadShapeParam.width / minScale;
		mVecCenterX[idx] = leadShapeParam.center_x / minScale;
		mVecCenterY[idx] = -leadShapeParam.center_y / minScale;

		std::vector<int> cutParam(leadShapeParam.number, 1);
		if (leadCutParam.size() != 0) {
			for (int cutNo = 0; cutNo < leadCutParam.size(); ++cutNo) {
				int beginIdx = leadCutParam[cutNo].position - 1;
				for (int idxOffset = 0; idxOffset < leadCutParam[cutNo].number; idxOffset++) {
					cutParam[beginIdx + idxOffset] = 0;
				}
			}
		}
		mVecCutParam[idx] = cutParam;
	}

	// 绘制源模板 无缩放 无旋转 模板超限保护
	std::vector<cv::Mat> leadSrcTpl;
	answer = GetLeadSourceTemplate(leadSrcTpl, mVecLeadWidth, mVecLeadLength, mVecLeadNum, mVecLeadPitch, mVecCutParam, mVecCenterX, mVecCenterY, 2, true, 0);
	if (answer.first != ALGErrCode::IMG_SUCCESS) return answer;

	float baseSplStep = 1;
	GetBaseSampleStep(leadSrcTpl, baseSplStep);

	// 设置匹配参数
	mStepNum = 3;
	/*float minWidth = 0.;
	for (int i = 0; i < mVecLeadWidth.size(); i++) {
		if (minWidth < 1 && mVecLeadWidth[i] >= 1) {
			minWidth = mVecLeadWidth[i];
		}
		if (minWidth >= 1 && mVecLeadWidth[i] >= 1 && mVecLeadWidth[i] < minWidth) {
			minWidth = mVecLeadWidth[i];
		}
	}*/
	//float maxWidth = 0.;
	//for (int i = 0; i < mVecLeadWidth.size(); i++) {
	//	if (maxWidth < mVecLeadWidth[i]) maxWidth = mVecLeadWidth[i];
	//}
	//if (maxWidth < 6) {
	//	mPyramidLevels = { 0, 0, 0 };
	//	mStepPixels = { 1, 1, 1 };
	//	mSobelSizes = { 3, 3, 3 };
	//	mSampleSteps = { baseSplStep * float(2), baseSplStep * float(2), baseSplStep * float(2) };
	//}
	//else if (maxWidth < 10) {
	//	mPyramidLevels = { 1, 0, 0 };
	//	mStepPixels = { 1, 1, 1 };
	//	mSobelSizes = { 3, 3, 3 };
	//	mSampleSteps = { baseSplStep * float(2), baseSplStep * float(2), baseSplStep * float(2) };
	//}
	//else {
		mPyramidLevels = { 2, 1, 0 };
		mStepPixels = { 1, 1, 1 };
		mSobelSizes = { 3, 3, 3 };
		mSampleSteps = { baseSplStep * mSplItv, baseSplStep * mSplItv, baseSplStep * mSplItv };
	//}


	//const float minAngle = atan2(1, std::max(mTotalX, mTotalY) * 0.5) / CV_PI * 180;
	//const float baseMinSplAngle = std::max(ceilf(minAngle * 10), float(1)) * float(0.1);
	//const float baseMidSplAngle = std::min(std::max(float(1), baseMinSplAngle * powf(2, abs(mPyramidLevels[1] - mPyramidLevels[2]) + 1)), float(1));
	//const float baseMaxSplAngle = baseMidSplAngle * powf(2, abs(mPyramidLevels[0] - mPyramidLevels[1]) + 1);
	//mStepAngles = { baseMaxSplAngle, baseMidSplAngle, baseMinSplAngle };
	//mStepAngles = { 3, 0.5, 0.1 };
	GetStepAngles(cv::Size2d(mTotalX, mTotalY), mPyramidLevels, mStepAngles);
	mAngleRange = mStepAngles[0];

	// 绘制引脚元件正模板
	mLeadStepTemplates.resize(mStepNum);
	for (int stepNo = 0; stepNo < mStepNum; ++stepNo) {
		GetLeadSourceTemplate(mLeadStepTemplates[stepNo], mVecLeadWidth, mVecLeadLength, mVecLeadNum, mVecLeadPitch, mVecCutParam, mVecCenterX, mVecCenterY, mSampleSteps[stepNo], stepNo == (mStepNum - 1), mPyramidLevels[stepNo]);
	}

	// 严格限制模板规模
	GetStrictSampleShapeTemplates(mLeadStepTemplates, mLeadStepTemplates);

# ifdef _DEBUG
	for (int i = 0; i < mLeadStepTemplates.size(); ++i) {
		CheckShapeTemplates(mLeadStepTemplates[i]);
	}
# endif

	// 绘制旋转模板
	GetRotatedShapeTemplates(mLeadStepTemplates[0], mLeadRotTemplatesCoarse, mBeginAngle + mAngleRange, mEndAngle - mAngleRange, mStepAngles[0]);
	GetScaleShapeTemplates(mLeadRotTemplatesCoarse, mScaleFactor);
	GetRotatedShapeTemplates(mLeadStepTemplates[1], mLeadRotTemplates, mBeginAngle, mEndAngle, mStepAngles[1]);
	GetScaleShapeTemplates(mLeadRotTemplates, mScaleFactor);


	// 绘制多尺寸模板
	std::vector<float> vecRatio = { 1.0 };
	mLeadMultiScaleTemplates.resize(vecRatio.size());
	std::vector<std::vector<double>> vecLeadWidthR(vecRatio.size());
	for (int i = 0; i < vecRatio.size(); ++i) {
		vecLeadWidthR[i].resize(mVecLeadWidth.size());
		for (int sideNo = 0; sideNo < mVecLeadWidth.size(); ++sideNo) {
			vecLeadWidthR[i][sideNo] = vecRatio[i] * mVecLeadWidth[sideNo];
		}
		GetLeadSourceTemplate(mLeadMultiScaleTemplates[i], vecLeadWidthR[i], mVecLeadLength, mVecLeadNum, mVecLeadPitch, mVecCutParam, mVecCenterX, mVecCenterY, mSampleSteps[2], true, mPyramidLevels[2]);
	}

	// 绘制单个引脚模板用于引脚结果检查
	GetSingleLeadSourceTemplate(mPinTemplates, mVecLeadWidth, mVecLeadLength, mVecLeadNum, mVecLeadPitch, mVecCutParam, mVecCenterX, mVecCenterY, 1, true, 0);
	GetScaleShapeTemplates(mPinTemplates, mScaleFactor);

#ifdef _DEBUG
	for (int i = 0; i < mPinTemplates.size(); ++i) {
		CheckShapeTemplates(mPinTemplates[i]);
	}
#endif

	return answer;
}

AnswerType CTemplateShapeLead::TemplateMatch(const cv::Mat& inSrcImg, cv::Point2f& outOffset, float& outAngle, float& outScore) {
	AnswerType answer = IMG_SUCCESS_ANS().SetErrCode();

	// 截图
	mPartImg = inSrcImg;

	// 匹配
	cv::Point2f offset;
	float angle = 0.;
	float score = 0.;
	cv::Mat cropImg, cropMagImg;
	std::vector<cv::Mat> cropGradImgs;
	cv::Point leftTop;
	float maxMagVal;
	cv::Mat crsMagImg;
	answer = PartDetect(cv::Point2f(mScaleX, mScaleY), mPartImg, mLeadStepTemplates, mLeadRotTemplatesCoarse, mLeadRotTemplates, mLeadMultiScaleTemplates, mPyramidLevels, mStepPixels, mBeginAngle, mEndAngle, mAngleRange, mStepAngles, mSobelSizes, cv::Size2f(float(mTotalX), float(mTotalY)), cv::Size2f(float(mMoldX), float(mMoldY)), mStepNum, offset, angle, score, crsMagImg, mCropImg, cropGradImgs, mCropMagImg, maxMagVal, mLeftTop, mScaleFactor);
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
	answer = GetPrecisePosition(mSrcImgCtr, mScaleFactor, mLeadMultiScaleTemplates[0], cv::Point2f(mScaleX, mScaleY), mPartImg.size(), mCropImg, mCropMagImg, cropGradImgs, mLeftTop, cv::Size2d(mTotalX, mTotalY), offset, angle);
	if (answer.first == ALGErrCode::IMG_SUCCESS) {
		std::vector<cv::Mat> tmpRotTpl;
		GetRotatedShapeTemplate(mLeadMultiScaleTemplates[0], tmpRotTpl, angle);
		GetScaleShapeTemplate(tmpRotTpl, mScaleFactor);
		const cv::Point2f imgCenter((mPartImg.cols - 1) * 0.5, (mPartImg.rows - 1) * 0.5);
		cv::Point2f rOffset(offset.x * pow(0.5, mPyramidLevels[2]) - mLeftTop.x + imgCenter.x, offset.y * pow(0.5, mPyramidLevels[2]) - mLeftTop.y + imgCenter.y);
		score = cm::calculateGradientScore(cropGradImgs[0], cropGradImgs[1], tmpRotTpl[0], tmpRotTpl[1], tmpRotTpl[2], tmpRotTpl[3], rOffset.x, rOffset.y) * 100;
#ifdef _DEBUG
		CheckMatchTemplate(mPartImg, mLeadMultiScaleTemplates[0], offset, angle, mPyramidLevels[2], 0, mScaleFactor);
		CheckMatchTemplateSVG(mPartImg, mLeadMultiScaleTemplates[0], offset, angle, mPyramidLevels[2], 0, mScaleFactor);
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

AnswerType CTemplateShapeLead::SaveResult(const cv::Mat& inSrcImg, const cv::Point2f& inOffset, const float& inAngle, const float& inScore, const AnswerType& inAnswer, const double& inTime, const std::string& inPath) {
	AnswerType answer = IMG_SUCCESS_ANS().SetErrCode();

	std::vector<cv::Mat> inSrcTpl = mLeadMultiScaleTemplates[0];
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
	std::vector<SVGItem> vecLeadSrcTpl;
	GetLeadSourceItem(vecLeadSrcTpl, mVecLeadWidth, mVecLeadLength, mVecLeadNum, mVecLeadPitch, mVecCutParam, mVecCenterX, mVecCenterY);
	GetRotatedItems(vecLeadSrcTpl, vecLeadSrcTpl, inAngle);
	GetTranslatedItems(vecLeadSrcTpl, vecLeadSrcTpl, offset - imgOfs + cv::Point2f(0.5, 0.5));
	for (int i = 0; i < vecLeadSrcTpl.size(); ++i) {
		st.drawRect(cv::Point2f(vecLeadSrcTpl[i].cx, vecLeadSrcTpl[i].cy), cv::Size2f(vecLeadSrcTpl[i].w, vecLeadSrcTpl[i].h), vecLeadSrcTpl[i].r, 2, "red");
	}
	/*st.drawRect(offset - imgOfs + cv::Point2f(0.5, 0.5), cv::Size2f(mTotalX, mTotalY), inAngle ,1, "red");*/

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
AnswerType CTemplateShapeLead::GetLeadItem(const cv::Size2f inLeadSize, SVGItem& outLeadTpl) {
	AnswerType answer = IMG_SUCCESS_ANS().SetErrCode();

	outLeadTpl = SVGItem(0, inLeadSize.height * 0.5, inLeadSize.width, inLeadSize.height, 0);

	return answer;
}

AnswerType CTemplateShapeLead::GetLeadGroupItem(const SVGItem& inLeadTpl, std::vector<SVGItem>& outLeadGroupTpl, const int& inLeadNum, const float& inLeadPitch, const std::vector<int>& inCutParam, const float& inAng, const cv::Point2f& inLeadCenter) {
	AnswerType answer = IMG_SUCCESS_ANS().SetErrCode();

	//std::vector<int> cutParam(inCutParam.begin(), inCutParam.end());
	//if (inLeadNum > 5) {
	//	int beginIdx = 0;
	//	for (int i = 0; i < cutParam.size(); ++i) {
	//		if (inCutParam[i] == 1) {
	//			beginIdx = i;
	//			break;
	//		}
	//	}
	//	int endIdx = 0;
	//	for (int i = cutParam.size() - 1; i > -1; --i) {
	//		if (inCutParam[i] == 1) {
	//			endIdx = i;
	//			break;
	//		}
	//	}
	//	int j = 0;
	//	for (int i = beginIdx; i < endIdx; ++i) {
	//		if (cutParam[i] == 1) {
	//			++j;
	//			if (j % 2 == 0) {
	//				cutParam[i] = 0;
	//			}
	//		}
	//	}
	//}

	const int leadNum = std::accumulate(inCutParam.begin(), inCutParam.end(), 0);
	SVGItem tmpTpl;
	std::vector<SVGItem> tmpTpls;

	const float beginX = -0.5 * (inLeadNum - 1) * inLeadPitch;
	int leadNo = -1;
	for (int i = 0; i < inLeadNum; ++i) {
		if (inCutParam[i] == 1) {
			++leadNo;
			GetTranslatedItem(inLeadTpl, tmpTpl, cv::Point2f(beginX + i * inLeadPitch, 0));
			tmpTpls.push_back(tmpTpl);
		}
	}

	//GetMergedShapeTemplate(tmpTpls, outLeadGroupTpl);

	GetRotatedItems(tmpTpls, outLeadGroupTpl, inAng);

	GetTranslatedItems(outLeadGroupTpl, outLeadGroupTpl, inLeadCenter);

	return answer;
}

AnswerType CTemplateShapeLead::GetLeadSourceItem(std::vector<SVGItem>& outLeadCompTpl, const std::vector<double>& inLeadWidths, const std::vector<double>& inLeadLengths, const std::vector<int>& inLeadNums, const std::vector<double>& inLeadPitches, const std::vector<std::vector<int>>& inCutParams, const std::vector<double>& inCenterXs, const std::vector<double>& inCenterYs) {
	AnswerType answer = IMG_SUCCESS_ANS().SetErrCode();

	std::vector<int> drawFlag(4, 0);
	for (int sideNo = 0; sideNo < 4; ++sideNo) {
		if (std::accumulate(inCutParams[sideNo].begin(), inCutParams[sideNo].end(), 0) != 0) drawFlag[sideNo] = 1;
	}
	bool singleSideFlag = false;
	if (std::accumulate(drawFlag.begin(), drawFlag.end(), 0) == 1) singleSideFlag = true;

	std::vector<SVGItem> leadTemplates(4);
	std::vector<std::vector<SVGItem>> leadGroupTemplates(4);
	for (int sideNo = 0; sideNo < 4; ++sideNo) {
		const int leadNum = std::accumulate(inCutParams[sideNo].begin(), inCutParams[sideNo].end(), 0);
		if (leadNum != 0) {
			//if (leadNum <= 2 || singleSideFlag) {
			GetLeadItem(cv::Size2f(inLeadWidths[sideNo], inLeadLengths[sideNo]), leadTemplates[sideNo]);
			//}
			//else {
			//	GetLeadTemplateFix(cv::Size2f(inLeadWidths[sideNo] * powf(0.5, inPyrLvl), inLeadLengths[sideNo] * powf(0.5, inPyrLvl)), leadTemplates[sideNo]);
			//}

			GetLeadGroupItem(leadTemplates[sideNo], leadGroupTemplates[sideNo], inLeadNums[sideNo], inLeadPitches[sideNo], inCutParams[sideNo], (90 - mMapIdxToAngle.find(sideNo)->second), cv::Point2f(inCenterXs[sideNo], inCenterYs[sideNo]));
		}
	}
	GetMergedItems(leadGroupTemplates, outLeadCompTpl);

	return answer;
}

// 模板绘制
AnswerType CTemplateShapeLead::GetLeadTemplate(const cv::Size2f inLeadSize, std::vector<cv::Mat>& outLeadTpl, const bool& inSingleSideFlag, const bool& inPreciseFlag, const float& inSplStep) {
	AnswerType answer = IMG_SUCCESS_ANS().SetErrCode();

	const float corRad = std::min(inLeadSize.width, inLeadSize.height) * 0.1;
	const float pinRatio = ((inLeadSize.height > inLeadSize.width * 1.2) && (!inSingleSideFlag || inPreciseFlag)) ? 0.4 : 0.6;  // 0.9
	const float tLeadWidth = inLeadSize.width;
	const float tLeadLength = inLeadSize.height * pinRatio;
	const float perimeter = CV_PI * corRad + tLeadWidth + 2 * tLeadLength - 4 * corRad;
	const int ptNum = std::max(int(perimeter / inSplStep) + 1, 3);
	const float splStep = perimeter / float(ptNum - 1);

	cv::Mat tX = cv::Mat::zeros(cv::Size(ptNum, 1), CV_32FC1);
	cv::Mat tY = cv::Mat::zeros(cv::Size(ptNum, 1), CV_32FC1);
	cv::Mat tGX = cv::Mat::zeros(cv::Size(ptNum, 1), CV_32FC1);
	cv::Mat tGY = cv::Mat::zeros(cv::Size(ptNum, 1), CV_32FC1);
	float* pX = tX.ptr<float>();
	float* pY = tY.ptr<float>();
	float* pGX = tGX.ptr<float>();
	float* pGY = tGY.ptr<float>();
	float distance = 0, angle = 0;
	for (int ptNo = 0; ptNo < ptNum; ++ptNo) {
		distance = ptNo * splStep;

		if (distance < tLeadLength - corRad) {  // 左
			pX[ptNo] = (-0.5 * tLeadWidth);
			pY[ptNo] = (tLeadLength)-(distance);
			pGX[ptNo] = 1;
			pGY[ptNo] = 0;
		}
		else if (distance < tLeadLength - corRad + 0.5 * CV_PI * corRad) {  // 左上
			angle = (distance - (tLeadLength - corRad)) / (0.5 * CV_PI * corRad) * 90 * CV_PI / 180.;
			pX[ptNo] = (-0.5 * inLeadSize.width + corRad) - (corRad * cosf(angle));
			pY[ptNo] = (corRad)-(corRad * sinf(angle));
			pGX[ptNo] = cosf(angle);
			pGY[ptNo] = sinf(angle);
		}
		else if (distance < tLeadWidth + tLeadLength - 3 * corRad + 0.5 * CV_PI * corRad) {  // 上
			pX[ptNo] = (-0.5 * tLeadWidth + corRad) + (distance - (tLeadLength - corRad + 0.5 * CV_PI * corRad));
			pY[ptNo] = (0);
			pGX[ptNo] = 0;
			pGY[ptNo] = 1;
		}
		else if (distance < tLeadWidth + tLeadLength - 3 * corRad + CV_PI * corRad) {  // 右上
			angle = (distance - (tLeadWidth + tLeadLength - 3 * corRad + 0.5 * CV_PI * corRad)) / (0.5 * CV_PI * corRad) * 90 * CV_PI / 180.;
			pX[ptNo] = (0.5 * tLeadWidth - corRad) + (corRad * sinf(angle));
			pY[ptNo] = (corRad)-(corRad * cosf(angle));
			pGX[ptNo] = -sinf(angle);
			pGY[ptNo] = cosf(angle);
		}
		else {  // 右
			pX[ptNo] = (0.5 * inLeadSize.width);
			pY[ptNo] = (corRad)+(distance - (tLeadWidth + tLeadLength - 3 * corRad + CV_PI * corRad));
			pGX[ptNo] = -1;
			pGY[ptNo] = 0;
		}
	}


	cv::Mat tVX, tVY;
	///*
	const int ptNumVY = std::max((int)round(inLeadSize.height * 0.8 / float(inSplStep * 2.) + 1.), 2);
	const float splStepVY = inLeadSize.height * 0.8 / float(ptNumVY - 1);

	tVX = cv::Mat::zeros(cv::Size(ptNumVY, 1), CV_32FC1);
	tVY = cv::Mat::zeros(cv::Size(ptNumVY, 1), CV_32FC1);
	float* pVX = tVX.ptr<float>();
	float* pVY = tVY.ptr<float>();
	for (int ptNo = 0; ptNo < ptNumVY; ++ptNo) {
		pVX[ptNo] = 0;
		pVY[ptNo] = 0.1 * inLeadSize.height + ptNo * splStepVY;
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

AnswerType CTemplateShapeLead::GetLeadTemplateX(const cv::Size2f inLeadSize, std::vector<cv::Mat>& outLeadTpl, const bool& inSingleSideFlag, const bool& inPreciseFlag, const float& inSplStep) {
	AnswerType answer = IMG_SUCCESS_ANS().SetErrCode();

	const float tLeadWidth = inLeadSize.width, tLeadLength = inLeadSize.height;
	const float perimeter = 2 * (tLeadWidth + tLeadLength);
	const int ptNum = std::max(int(perimeter / inSplStep) + 1, 3);
	const float splStep = perimeter / float(ptNum - 1);

	cv::Mat tX = cv::Mat::zeros(cv::Size(ptNum, 1), CV_32FC1);
	cv::Mat tY = cv::Mat::zeros(cv::Size(ptNum, 1), CV_32FC1);
	cv::Mat tGX = cv::Mat::zeros(cv::Size(ptNum, 1), CV_32FC1);
	cv::Mat tGY = cv::Mat::zeros(cv::Size(ptNum, 1), CV_32FC1);
	float* pX = tX.ptr<float>();
	float* pY = tY.ptr<float>();
	float* pGX = tGX.ptr<float>();
	float* pGY = tGY.ptr<float>();
	float distance = 0;
	for (int ptNo = 0; ptNo < ptNum; ++ptNo) {
		distance = ptNo * splStep;

		if (distance < tLeadLength) {  // 左
			pX[ptNo] = (-0.5 * tLeadWidth);
			pY[ptNo] = (tLeadLength)-(distance);
			pGX[ptNo] = 1;
			pGY[ptNo] = 0;
		}
		else if (distance < tLeadLength + tLeadWidth) {  // 上
			pX[ptNo] = (- 0.5 * inLeadSize.width) + (distance - tLeadLength);
			pY[ptNo] = 0;
			pGX[ptNo] = 0;
			pGY[ptNo] = 1;
		}
		else if (distance < 2 * tLeadLength + tLeadWidth) {  // 右
			pX[ptNo] = 0.5 * inLeadSize.width;
			pY[ptNo] = 0 + (distance - (tLeadLength + tLeadWidth));
			pGX[ptNo] = -1;
			pGY[ptNo] = 0;
		}
		else {  // 下
			pX[ptNo] = (0.5 * inLeadSize.width) - (distance - (2 * tLeadLength + tLeadWidth));
			pY[ptNo] = tLeadLength;
			pGX[ptNo] = -1;
			pGY[ptNo] = 0;
		}
	}


	cv::Mat tVX, tVY;
	///*
	const int ptNumVY = std::max((int)round(inLeadSize.height * 0.8 / float(inSplStep * 2.) + 1.), 2);
	const float splStepVY = inLeadSize.height * 0.8 / float(ptNumVY - 1);

	tVX = cv::Mat::zeros(cv::Size(ptNumVY, 1), CV_32FC1);
	tVY = cv::Mat::zeros(cv::Size(ptNumVY, 1), CV_32FC1);
	float* pVX = tVX.ptr<float>();
	float* pVY = tVY.ptr<float>();
	for (int ptNo = 0; ptNo < ptNumVY; ++ptNo) {
		pVX[ptNo] = 0;
		pVY[ptNo] = 0.1 * inLeadSize.height + ptNo * splStepVY;
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

AnswerType CTemplateShapeLead::GetLeadTemplateFix(const cv::Size2f inLeadSize, std::vector<cv::Mat>& outLeadTpl) {
	AnswerType answer = IMG_SUCCESS_ANS().SetErrCode();

	const float corRad = std::min(inLeadSize.width, inLeadSize.height) * 0.1;
	const float leadWidth = inLeadSize.width;
	const float leadHeight = inLeadSize.height;
	const int ptNum = 7;

	cv::Mat tX = cv::Mat::zeros(cv::Size(ptNum, 1), CV_32FC1);
	cv::Mat tY = cv::Mat::zeros(cv::Size(ptNum, 1), CV_32FC1);
	cv::Mat tGX = cv::Mat::zeros(cv::Size(ptNum, 1), CV_32FC1);
	cv::Mat tGY = cv::Mat::zeros(cv::Size(ptNum, 1), CV_32FC1);
	float* pX = tX.ptr<float>();
	float* pY = tY.ptr<float>();
	float* pGX = tGX.ptr<float>();
	float* pGY = tGY.ptr<float>();

	int leftPtNum = 0, upPtNum = 0, rightPtNum = 0;
	float pinRatio = 0;
	const float scale = leadWidth / leadHeight;
	if (scale < 0.63) {
		leftPtNum = 3; upPtNum = 1; rightPtNum = 3;
		pinRatio = 0.5;
	}
	else if (scale < 2) {
		leftPtNum = 2; upPtNum = 3; rightPtNum = 2;
		pinRatio = 0.8;
	}
	else {
		leftPtNum = 1; upPtNum = 5; rightPtNum = 1;
		pinRatio = 1.0;
	}

	// 左侧
	const float leftLeadHeight = leadHeight * pinRatio;
	const float leftSplLen = leftLeadHeight / leftPtNum;
	const float leftBeginY = leftLeadHeight - 0.5 * leftSplLen;
	for (int i = 0; i < leftPtNum; ++i) {
		pX[i] = -0.5 * leadWidth;
		pY[i] = leftBeginY - i * leftSplLen;
		pGX[i] = 1;
		pGY[i] = 0;
	}

	// 上侧
	const float upSplLen = leadWidth / upPtNum;
	const float upBeginX = -0.5 * leadWidth + 0.5 * upSplLen;
	for (int i = 0; i < upPtNum; ++i) {
		pX[i + leftPtNum] = upBeginX + i * upSplLen;
		pY[i + leftPtNum] = 0;
		pGX[i + leftPtNum] = 0;
		pGY[i + leftPtNum] = 1;
	}

	// 右侧
	const float rightLeadHeight = leadHeight * pinRatio;
	const float rightSplLen = rightLeadHeight / rightPtNum;
	const float rightBeginY = 0.5 * rightSplLen;
	for (int i = 0; i < rightPtNum; ++i) {
		pX[i + leftPtNum + upPtNum] = 0.5 * leadWidth;
		pY[i + leftPtNum + upPtNum] = rightBeginY + i * rightSplLen;
		pGX[i + leftPtNum + upPtNum] = -1;
		pGY[i + leftPtNum + upPtNum] = 0;
	}

	outLeadTpl = { tX, tY, tGX, tGY };

	return answer;
}

AnswerType CTemplateShapeLead::GetLeadGroupTemplate(const std::vector<cv::Mat>& inLeadTpl, std::vector<cv::Mat>& outLeadGroupTpl, const int& inLeadNum, const float& inLeadPitch, const std::vector<int>& inCutParam, const float& inAng, const cv::Point2f& inLeadCenter) {
	AnswerType answer = IMG_SUCCESS_ANS().SetErrCode();

	//std::vector<int> cutParam(inCutParam.begin(), inCutParam.end());
	//if (inLeadNum > 5) {
	//	int beginIdx = 0;
	//	for (int i = 0; i < cutParam.size(); ++i) {
	//		if (inCutParam[i] == 1) {
	//			beginIdx = i;
	//			break;
	//		}
	//	}
	//	int endIdx = 0;
	//	for (int i = cutParam.size() - 1; i > -1; --i) {
	//		if (inCutParam[i] == 1) {
	//			endIdx = i;
	//			break;
	//		}
	//	}
	//	int j = 0;
	//	for (int i = beginIdx; i < endIdx; ++i) {
	//		if (cutParam[i] == 1) {
	//			++j;
	//			if (j % 2 == 0) {
	//				cutParam[i] = 0;
	//			}
	//		}
	//	}
	//}

	const int leadNum = std::accumulate(inCutParam.begin(), inCutParam.end(), 0);
	const int tplSize = leadNum * inLeadTpl[0].cols;
	std::vector<cv::Mat> tmpTpl;
	std::vector<std::vector<cv::Mat>> tmpTpls;

	const float beginX = -0.5 * (inLeadNum - 1) * inLeadPitch;
	int leadNo = -1;
	for (int i = 0; i < inLeadNum; ++i) {
		if (inCutParam[i] == 1) {
			++leadNo;
			GetTranslatedShapeTemplate(inLeadTpl, tmpTpl, cv::Point2f(beginX + i * inLeadPitch, 0));
			tmpTpls.push_back(tmpTpl);
		}
	}

	GetMergedShapeTemplate(tmpTpls, outLeadGroupTpl);

	GetRotatedShapeTemplate(outLeadGroupTpl, outLeadGroupTpl, inAng);

	GetTranslatedShapeTemplate(outLeadGroupTpl, outLeadGroupTpl, inLeadCenter);

	return answer;
}

AnswerType CTemplateShapeLead::GetLeadSourceTemplate(std::vector<cv::Mat>& outLeadCompTpl, const std::vector<double>& inLeadWidths, const std::vector<double>& inLeadLengths, const std::vector<int>& inLeadNums, const std::vector<double>& inLeadPitches, const std::vector<std::vector<int>>& inCutParams, const std::vector<double>& inCenterXs, const std::vector<double>& inCenterYs, const float& inSplStep, const bool& inPreciseFlag, const int& inPyrLvl) {
	AnswerType answer = IMG_SUCCESS_ANS().SetErrCode();

	std::vector<int> drawFlag(4, 0);
	for (int sideNo = 0; sideNo < 4; ++sideNo) {
		if (std::accumulate(inCutParams[sideNo].begin(), inCutParams[sideNo].end(), 0) != 0) drawFlag[sideNo] = 1;
	}
	bool singleSideFlag = false;
	if (std::accumulate(drawFlag.begin(), drawFlag.end(), 0) == 1) singleSideFlag = true;

	std::vector<std::vector<cv::Mat>> leadTemplates(4);
	std::vector<std::vector<cv::Mat>> leadGroupTemplates(4);
	for (int sideNo = 0; sideNo < 4; ++sideNo) {
		const int leadNum = std::accumulate(inCutParams[sideNo].begin(), inCutParams[sideNo].end(), 0);
		if (leadNum != 0) {
			//if (leadNum <= 2 || singleSideFlag) {
			GetLeadTemplate(cv::Size2f(inLeadWidths[sideNo] * powf(0.5, inPyrLvl), inLeadLengths[sideNo] * powf(0.5, inPyrLvl)), leadTemplates[sideNo], singleSideFlag, inPreciseFlag, inSplStep);
			//}
			//else {
			//	GetLeadTemplateFix(cv::Size2f(inLeadWidths[sideNo] * powf(0.5, inPyrLvl), inLeadLengths[sideNo] * powf(0.5, inPyrLvl)), leadTemplates[sideNo]);
			//}

			GetLeadGroupTemplate(leadTemplates[sideNo], leadGroupTemplates[sideNo], inLeadNums[sideNo], inLeadPitches[sideNo] * powf(0.5, inPyrLvl), inCutParams[sideNo], (90 - mMapIdxToAngle.find(sideNo)->second), cv::Point2f(inCenterXs[sideNo] * powf(0.5, inPyrLvl), inCenterYs[sideNo] * powf(0.5, inPyrLvl)));
		}
	}
	GetMergedShapeTemplate(leadGroupTemplates, outLeadCompTpl);

	return answer;
}

AnswerType CTemplateShapeLead::GetSingleLeadGroupTemplate(const std::vector<cv::Mat>& inLeadTpl, std::vector<std::vector<cv::Mat>>& outPinGroupTpl, const int& inLeadNum, const float& inLeadPitch, const std::vector<int>& inCutParam, const float& inAng, const cv::Point2f& inLeadCenter) {
	AnswerType answer = IMG_SUCCESS_ANS().SetErrCode();

	const int leadNum = std::accumulate(inCutParam.begin(), inCutParam.end(), 0);
	const int tplSize = leadNum * inLeadTpl[0].rows;
	std::vector<cv::Mat> tmpTpl;
	std::vector<std::vector<cv::Mat>> tmpTpls;

	const float beginX = -0.5 * (inLeadNum - 1) * inLeadPitch;
	int leadNo = -1;
	for (int i = 0; i < inLeadNum; ++i) {
		if (inCutParam[i] == 1) {
			++leadNo;
			GetTranslatedShapeTemplate(inLeadTpl, tmpTpl, cv::Point2f(beginX + i * inLeadPitch, 0));
			tmpTpls.push_back(tmpTpl);
		}
	}

	for (int i = 0; i < tmpTpls.size(); ++i) {
		GetRotatedShapeTemplate(tmpTpls[i], tmpTpls[i], inAng);

		GetTranslatedShapeTemplate(tmpTpls[i], tmpTpls[i], inLeadCenter);
	}

	outPinGroupTpl = tmpTpls;

	return answer;
}

AnswerType CTemplateShapeLead::GetSingleLeadSourceTemplate(std::vector<std::vector<cv::Mat>>& outPinCompTpl, const std::vector<double>& inLeadWidths, const std::vector<double>& inLeadLengths, const std::vector<int>& inLeadNums, const std::vector<double>& inLeadPitches, const std::vector<std::vector<int>>& inCutParams, const std::vector<double>& inCenterXs, const std::vector<double>& inCenterYs, const float& inSplStep, const bool& inPreciseFlag, const int& inPyrLvl) {
	AnswerType answer = IMG_SUCCESS_ANS().SetErrCode();

	std::vector<int> drawFlag(4, 0);
	for (int sideNo = 0; sideNo < 4; ++sideNo) {
		if (std::accumulate(inCutParams[sideNo].begin(), inCutParams[sideNo].end(), 0) != 0) drawFlag[sideNo] = 1;
	}
	bool singleSideFlag = false;
	if (std::accumulate(drawFlag.begin(), drawFlag.end(), 0) == 1) singleSideFlag = true;

	std::vector<std::vector<cv::Mat>> pinTemplates(4);
	std::vector<std::vector<std::vector<cv::Mat>>> pinGroupTemplates(4);
	for (int sideNo = 0; sideNo < 4; ++sideNo) {
		if (std::accumulate(inCutParams[sideNo].begin(), inCutParams[sideNo].end(), 0) != 0) {
			GetLeadTemplate(cv::Size2f(inLeadWidths[sideNo] * powf(0.5, inPyrLvl), inLeadLengths[sideNo] * powf(0.5, inPyrLvl)), pinTemplates[sideNo], singleSideFlag, inPreciseFlag, inSplStep);

			GetSingleLeadGroupTemplate(pinTemplates[sideNo], pinGroupTemplates[sideNo], inLeadNums[sideNo], inLeadPitches[sideNo] * powf(0.5, inPyrLvl), inCutParams[sideNo], (90 - mMapIdxToAngle.find(sideNo)->second), cv::Point2f(inCenterXs[sideNo] * powf(0.5, inPyrLvl), inCenterYs[sideNo] * powf(0.5, inPyrLvl)));
		}
	}

	std::vector<std::vector<cv::Mat>> allPinTemplates;
	for (int sideNo = 0; sideNo < 4; ++sideNo) {
		for (int i = 0; i < pinGroupTemplates[sideNo].size(); ++i) {
			allPinTemplates.push_back(pinGroupTemplates[sideNo][i]);
		}
	}

	outPinCompTpl = allPinTemplates;

	return answer;
}
