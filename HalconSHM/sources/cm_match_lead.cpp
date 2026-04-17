#include "cm_match_lead.hpp"


using namespace cm;

CTemplateShapeLead::CTemplateShapeLead(float inScaleX, float inScaleY) : CTemplatePartGroup(inScaleX, inScaleY) {
	mTotalX = 0;
	mTotalY = 0;
	mMoldX = 0;
	mMoldY = 0;
}

AnswerType CTemplateShapeLead::GenerateTemplate(std::shared_ptr<Component> inCompPtr) {
    AnswerType answer = IMG_SUCCESS_ANS().SetErrCode();

	/***   获取参数   ***/
	const float meanScale = (mScaleX + mScaleY) * 0.5;
    mTotalX = inCompPtr->GetCommonData().GetComponentLenth() / meanScale;
    mTotalY = inCompPtr->GetCommonData().GetComponentWidth() / meanScale;

	std::shared_ptr<AbstractConfigurationBasic> pAbsLead = std::dynamic_pointer_cast<AbstractConfigurationBasic>(inCompPtr->GetOneAbstractConfigurationBasic(ConfigurationBasicType::LEAD_GROUP));
	std::shared_ptr<AbstractConfigurationBasic> pAbsMold = std::dynamic_pointer_cast<AbstractConfigurationBasic>(inCompPtr->GetOneAbstractConfigurationBasic(ConfigurationBasicType::MOLD));
	
	std::shared_ptr<LeadGroupBasic> pLead = std::dynamic_pointer_cast<LeadGroupBasic>(pAbsLead);
	std::shared_ptr<MoldBasic> pMold = std::dynamic_pointer_cast<MoldBasic>(pAbsMold);
	MoldParam moldParam = pMold->GetParam();

	mMoldX = moldParam.GetShapeParam().length / meanScale;
	mMoldY = moldParam.GetShapeParam().width / meanScale;

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
		mVecLeadPitch[idx] = leadShapeParam.pitch / meanScale;
		mVecLeadLength[idx] = leadShapeParam.length / meanScale;
		mVecLeadWidth[idx] = leadShapeParam.width / meanScale;
		mVecCenterX[idx] = leadShapeParam.center_x / meanScale;
		mVecCenterY[idx] = -leadShapeParam.center_y / meanScale;

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

	/***   绘制模板   ***/
	//// 计算模板尺寸
	//float tmpMaxAbsX = 0, tmpMaxAbsY = 0;
	//for (int i = 0; i < mVecCenterX.size(); ++i) {
	//	if (ceil(abs(mVecCenterX[i])) > tmpMaxAbsX) {
	//		tmpMaxAbsX = ceil(abs(mVecCenterX[i]));
	//	}
	//	if (ceil(abs(mVecCenterY[i])) > tmpMaxAbsY) {
	//		tmpMaxAbsY = ceil(abs(mVecCenterY[i]));
	//	}
	//}
	//mTplSize = cv::Size((tmpMaxAbsX + mBorWidth) * 2, (tmpMaxAbsY + mBorWidth) * 2);
	const int maxSize = ceil(max(mTotalX, mTotalY));
	mTplSize = cv::Size(maxSize + mBorWidth * 2, maxSize + mBorWidth * 2);
	mTplCtr = cv::Point2f((mTplSize.width - 1) * 0.5, (mTplSize.height - 1) * 0.5);

	HalconCpp::HImage Image;
	Image.GenImageConst("byte", mTplSize.width, mTplSize.height);

	HalconCpp::HObject leadSrcTpl;
	GetLeadSourceTemplate(leadSrcTpl, mVecLeadWidth, mVecLeadLength, mVecLeadNum, mVecLeadPitch, mVecCutParam, mVecCenterX, mVecCenterY);

	// 创建Shape模型
	CreateShapeModelXld(leadSrcTpl,
		mPyramidLevels,
		DegToRad(-30),
		DegToRad(60),
		"auto", "auto", "ignore_local_polarity",
		5, &mModelID);

#ifdef _DEBUG
	HalconCpp::HObject h1;
	GetTranslatedShapeTemplate(leadSrcTpl, h1, mTplCtr);
	VisualizeHObjectWin(h1, mTplSize);
#endif

    return answer;
}

AnswerType CTemplateShapeLead::GetLeadTemplate(const cv::Size2f& inLeadSize, HalconCpp::HObject& outLeadTpl) {
	AnswerType answer = IMG_SUCCESS_ANS().SetErrCode();

	HalconCpp::HRegion hLead;
	GenRectangle2ContourXld(&hLead, inLeadSize.height * 0.5, 0, 0.0, inLeadSize.width * 0.5, inLeadSize.height * 0.5);

//#ifdef _DEBUG
//	VisualizeHObjectWin(hLead, mTplSize);
//#endif

	outLeadTpl = hLead;

	return answer;
}

AnswerType CTemplateShapeLead::GetLeadGroupTemplate(const HalconCpp::HObject& inLeadTpl, HalconCpp::HObject& outLeadGroupTpl, const int& inLeadNum, const float& inLeadPitch, const std::vector<int>& inCutParam, const float& inAng, const cv::Point2f& inLeadCenter) {
	AnswerType answer = IMG_SUCCESS_ANS().SetErrCode();

	const int leadNum = std::accumulate(inCutParam.begin(), inCutParam.end(), 0);
	std::vector<HalconCpp::HObject> tmpTpls;

	const float beginX = -0.5 * (inLeadNum - 1) * inLeadPitch;
	int leadNo = -1;
	for (int i = 0; i < inLeadNum; ++i) {
		if (inCutParam[i] == 1) {
			++leadNo;
			HalconCpp::HObject tmpTpl;
			GetTranslatedShapeTemplate(inLeadTpl, tmpTpl, cv::Point2f(beginX + i * inLeadPitch, 0));
			tmpTpls.push_back(tmpTpl);
		}
	}
	
	GetMergedShapeTemplate(tmpTpls, outLeadGroupTpl);

//#ifdef _DEBUG
//	VisualizeHObjectWin(outLeadGroupTpl, mTplSize);
//#endif

	GetRotatedShapeTemplate(outLeadGroupTpl, outLeadGroupTpl, cv::Point2f(0, 0), inAng);

//#ifdef _DEBUG
//	VisualizeHObjectWin(outLeadGroupTpl, mTplSize);
//#endif

	GetTranslatedShapeTemplate(outLeadGroupTpl, outLeadGroupTpl, inLeadCenter);

//#ifdef _DEBUG
//	VisualizeHObjectWin(outLeadGroupTpl, mTplSize);
//#endif

	return answer;
}

AnswerType CTemplateShapeLead::GetLeadSourceTemplate(HalconCpp::HObject& outLeadCompTpl, const std::vector<double>& inLeadWidths, const std::vector<double>& inLeadLengths, const std::vector<int>& inLeadNums, const std::vector<double>& inLeadPitches, const std::vector<std::vector<int>>& inCutParams, const std::vector<double>& inCenterXs, const std::vector<double>& inCenterYs) {
	AnswerType answer = IMG_SUCCESS_ANS().SetErrCode();

	std::vector<int> drawFlag(4, 0);
	for (int sideNo = 0; sideNo < 4; ++sideNo) {
		if (std::accumulate(inCutParams[sideNo].begin(), inCutParams[sideNo].end(), 0) != 0) drawFlag[sideNo] = 1;
	}
	bool singleSideFlag = false;
	if (std::accumulate(drawFlag.begin(), drawFlag.end(), 0) == 1) singleSideFlag = true;

	//std::vector<HalconCpp::HObject> leadTemplates(4);
	std::vector<HalconCpp::HObject> leadGroupTemplates;
	for (int sideNo = 0; sideNo < 4; ++sideNo) {
		const int leadNum = std::accumulate(inCutParams[sideNo].begin(), inCutParams[sideNo].end(), 0);
		if (leadNum != 0) {
			//if (leadNum <= 2 || singleSideFlag) {
			HalconCpp::HObject leadTemplate;
			GetLeadTemplate(cv::Size2f(inLeadWidths[sideNo], inLeadLengths[sideNo]), leadTemplate);
			//}
			//else {
			//	GetLeadTemplateFix(cv::Size2f(inLeadWidths[sideNo] * powf(0.5, inPyrLvl), inLeadLengths[sideNo] * powf(0.5, inPyrLvl)), leadTemplates[sideNo]);
			//}

			HalconCpp::HObject leadGroupTemplate;
			GetLeadGroupTemplate(leadTemplate, leadGroupTemplate, inLeadNums[sideNo], inLeadPitches[sideNo], inCutParams[sideNo], (90 - mMapIdxToAngle.find(sideNo)->second), cv::Point2f(inCenterXs[sideNo], inCenterYs[sideNo]));

			leadGroupTemplates.push_back(leadGroupTemplate);
		}
	}
	GetMergedShapeTemplate(leadGroupTemplates, outLeadCompTpl);

//#ifdef _DEBUG
//	HalconCpp::HObject h1;
//	GetTranslatedShapeTemplate(outLeadCompTpl, h1, mTplCtr);
//	VisualizeHObjectWin(h1, mTplSize);
//#endif

	return answer;
}
