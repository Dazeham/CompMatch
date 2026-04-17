#pragma once
#ifndef CM_MATCH_LEAD_HPP
#define CM_MATCH_LEAD_HPP
#include "cm_match.hpp"
#include "cm_svg.hpp"


namespace cm {
	class CTemplateShapeLead : public CTemplatePartGroup {
	public:
		CTemplateShapeLead() = delete;
		CTemplateShapeLead(float inScaleX, float inScaleY);
		~CTemplateShapeLead() {};
		AnswerType GenerateTemplate(std::shared_ptr<Component> inCompPtr);
		AnswerType TemplateMatch(const cv::Mat& inSrcImg, cv::Point2f& outOffset, float& outAngle, float& outScore);
		AnswerType SaveResult(const cv::Mat& inSrcImg, const cv::Point2f& inOffset, const float& inAngle, const float& inScore, const AnswerType& inAnswer, const double& inTime, const std::string& inPath);

		// 成员函数
	private:
		// SVG轮廓
		AnswerType GetLeadItem(const cv::Size2f inLeadSize, SVGItem& outLeadTpl);
		AnswerType GetLeadGroupItem(const SVGItem& inLeadTpl, std::vector<SVGItem>& outLeadGroupTpl, const int& inLeadNum, const float& inLeadPitch, const std::vector<int>& inCutParam, const float& inAng, const cv::Point2f& inLeadCenter);
		AnswerType GetLeadSourceItem(std::vector<SVGItem>& outLeadCompTpl, const std::vector<double>& inLeadWidths, const std::vector<double>& inLeadLengths, const std::vector<int>& inLeadNums, const std::vector<double>& inLeadPitches, const std::vector<std::vector<int>>& inCutParams, const std::vector<double>& inCenterXs, const std::vector<double>& inCenterYs);


		// 模板绘制
		AnswerType GetLeadTemplate(const cv::Size2f inLeadSize, std::vector<cv::Mat>& outLeadTpl, const bool& inSingleSideFlag, const bool& inPreciseFlag, const float& inSplStep);
		AnswerType CTemplateShapeLead::GetLeadTemplateX(const cv::Size2f inLeadSize, std::vector<cv::Mat>& outLeadTpl, const bool& inSingleSideFlag, const bool& inPreciseFlag, const float& inSplStep);
		AnswerType GetLeadTemplateFix(const cv::Size2f inLeadSize, std::vector<cv::Mat>& outLeadTpl);
		AnswerType GetLeadGroupTemplate(const std::vector<cv::Mat>& inLeadTpl, std::vector<cv::Mat>& outLeadGroupTpl, const int& inLeadNum, const float& inLeadPitch, const std::vector<int>& inCutParam, const float& inAng, const cv::Point2f& inLeadCenter);
		AnswerType GetLeadSourceTemplate(std::vector<cv::Mat>& outLeadCompTpl, const std::vector<double>& inLeadWidths, const std::vector<double>& inLeadLengths, const std::vector<int>& inLeadNums, const std::vector<double>& inLeadPitches, const std::vector<std::vector<int>>& inCutParams, const std::vector<double>& inCenterXs, const std::vector<double>& inCenterYs, const float& inSplStep, const bool& inPreciseFlag, const int& inPyrLvl);
		AnswerType GetSingleLeadGroupTemplate(const std::vector<cv::Mat>& inLeadTpl, std::vector<std::vector<cv::Mat>>& outPinGroupTpl, const int& inLeadNum, const float& inLeadPitch, const std::vector<int>& inCutParam, const float& inAng, const cv::Point2f& inLeadCenter);
		AnswerType GetSingleLeadSourceTemplate(std::vector<std::vector<cv::Mat>>& outPinCompTpl, const std::vector<double>& inLeadWidths, const std::vector<double>& inLeadLengths, const std::vector<int>& inLeadNums, const std::vector<double>& inLeadPitches, const std::vector<std::vector<int>>& inCutParams, const std::vector<double>& inCenterXs, const std::vector<double>& inCenterYs, const float& inSplStep, const bool& inPreciseFlag, const int& inPyrLvl);

		// 成员变量
	private:
		const std::map<double, int> mMapAngleToIdx = { {90, 0}, {0, 1}, {-90, 2}, {180, 3} };
		const std::map<int, double> mMapIdxToAngle = { {0, 90}, {1, 0}, {2, -90}, {3, 180} };

		// 元件参数
		cv::Point2d mScaleFactor;
		double mTotalX;
		double mTotalY;
		double mMoldX;
		double mMoldY;
		std::vector<int> mVecLeadNum;	           //引脚数目
		std::vector<double> mVecLeadPitch;	       //引脚间距
		std::vector<double> mVecLeadLength;          //引脚长度
		std::vector<double> mVecLeadWidth;	       //引脚宽度
		std::vector<double> mVecCenterX;             //引脚组中心x
		std::vector<double> mVecCenterY;             //引脚组中心y
		std::vector<std::vector<int>> mVecCutParam;  // 缺失引脚

		// 匹配参数
		int mStepNum;
		std::vector<float> mSampleSteps;
		std::vector<int> mPyramidLevels;
		std::vector<float> mStepPixels;
		std::vector<float> mStepAngles;
		std::vector<int> mSobelSizes;
		float mAngleRange;  // 普通匹配角度范围

		// 形状模板
		std::vector<std::vector<cv::Mat>> mLeadStepTemplates;
		std::vector<std::vector<cv::Mat>> mLeadRotTemplatesCoarse;
		std::vector<std::vector<cv::Mat>> mLeadRotTemplates;
		std::vector<std::vector<cv::Mat>> mLeadMultiScaleTemplates;

		// 结果检查
		cv::Mat mPartImg;
		cv::Point mLeftTop;
		std::vector<std::vector<cv::Mat>> mPinTemplates;
		cv::Point2f mSrcImgCtr;
		cv::Point2f mCropImgCtr;
		cv::Point2d mSclFac;
		cv::Mat mCropImg;
		cv::Mat mCropMagImg;
	};
}

#endif
