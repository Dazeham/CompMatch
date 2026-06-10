#pragma once
#ifndef CM_MATCH_LEAD_HPP
#define CM_MATCH_LEAD_HPP
#include "cm_match.hpp"
#include "cm_svg.hpp"


namespace cm {
	// Matches leaded components by composing lead-group templates.
	class CTemplateShapeLead : public CTemplatePartGroup {
	public:
		CTemplateShapeLead() = delete;
		CTemplateShapeLead(float inScaleX, float inScaleY);
		~CTemplateShapeLead() {};
		// Build lead templates from component and missing-lead metadata.
		AnswerType GenerateTemplate(std::shared_ptr<Component> inCompPtr);
		// Match a leaded component and return offset, angle, and score.
		AnswerType TemplateMatch(const cv::Mat& inSrcImg, cv::Point2f& outOffset, float& outAngle, float& outScore);
		// Save a lead match overlay as an SVG result.
		AnswerType SaveResult(const cv::Mat& inSrcImg, const cv::Point2f& inOffset, const float& inAngle, const float& inScore, const AnswerType& inAnswer, const double& inTime, const std::string& inPath);

		// Member functions
	private:
		// SVG contour
		// Build an SVG item for one lead.
		AnswerType GetLeadItem(const cv::Size2f inLeadSize, SVGItem& outLeadTpl);
		// Arrange one lead SVG item into a side group.
		AnswerType GetLeadGroupItem(const SVGItem& inLeadTpl, std::vector<SVGItem>& outLeadGroupTpl, const int& inLeadNum, const float& inLeadPitch, const std::vector<int>& inCutParam, const float& inAng, const cv::Point2f& inLeadCenter);
		// Combine all lead SVG groups into one component contour.
		AnswerType GetLeadSourceItem(std::vector<SVGItem>& outLeadCompTpl, const std::vector<double>& inLeadWidths, const std::vector<double>& inLeadLengths, const std::vector<int>& inLeadNums, const std::vector<double>& inLeadPitches, const std::vector<std::vector<int>>& inCutParams, const std::vector<double>& inCenterXs, const std::vector<double>& inCenterYs);


		// Template drawing
		// Draw the sampled contour for one lead.
		AnswerType GetLeadTemplate(const cv::Size2f inLeadSize, std::vector<cv::Mat>& outLeadTpl, const bool& inSingleSideFlag, const bool& inPreciseFlag, const float& inSplStep);
		// Draw a simplified sampled contour for one lead.
		AnswerType GetLeadTemplateS(const cv::Size2f inLeadSize, std::vector<cv::Mat>& outLeadTpl, const bool& inSingleSideFlag, const bool& inPreciseFlag, const float& inSplStep);
		// Draw an alternate sampled contour for one lead.
		AnswerType CTemplateShapeLead::GetLeadTemplateX(const cv::Size2f inLeadSize, std::vector<cv::Mat>& outLeadTpl, const bool& inSingleSideFlag, const bool& inPreciseFlag, const float& inSplStep);
		// Draw a fixed-shape lead template.
		AnswerType GetLeadTemplateFix(const cv::Size2f inLeadSize, std::vector<cv::Mat>& outLeadTpl);
		// Arrange one lead template into a side group.
		AnswerType GetLeadGroupTemplate(const std::vector<cv::Mat>& inLeadTpl, std::vector<cv::Mat>& outLeadGroupTpl, const int& inLeadNum, const float& inLeadPitch, const std::vector<int>& inCutParam, const float& inAng, const cv::Point2f& inLeadCenter);
		// Combine all lead groups into one component template.
		AnswerType GetLeadSourceTemplate(std::vector<cv::Mat>& outLeadCompTpl, const std::vector<double>& inLeadWidths, const std::vector<double>& inLeadLengths, const std::vector<int>& inLeadNums, const std::vector<double>& inLeadPitches, const std::vector<std::vector<int>>& inCutParams, const std::vector<double>& inCenterXs, const std::vector<double>& inCenterYs, const float& inSplStep, const bool& inPreciseFlag, const int& inPyrLvl);
		// Arrange one lead template into per-lead subtemplates.
		AnswerType GetSingleLeadGroupTemplate(const std::vector<cv::Mat>& inLeadTpl, std::vector<std::vector<cv::Mat>>& outPinGroupTpl, const int& inLeadNum, const float& inLeadPitch, const std::vector<int>& inCutParam, const float& inAng, const cv::Point2f& inLeadCenter);
		// Combine all per-lead subtemplates for detailed checking.
		AnswerType GetSingleLeadSourceTemplate(std::vector<std::vector<cv::Mat>>& outPinCompTpl, const std::vector<double>& inLeadWidths, const std::vector<double>& inLeadLengths, const std::vector<int>& inLeadNums, const std::vector<double>& inLeadPitches, const std::vector<std::vector<int>>& inCutParams, const std::vector<double>& inCenterXs, const std::vector<double>& inCenterYs, const float& inSplStep, const bool& inPreciseFlag, const int& inPyrLvl);

		// Member variables
	private:
		const std::map<double, int> mMapAngleToIdx = { {90, 0}, {0, 1}, {-90, 2}, {180, 3} };
		const std::map<int, double> mMapIdxToAngle = { {0, 90}, {1, 0}, {2, -90}, {3, 180} };

		// Component parameters
		cv::Point2d mScaleFactor;
		double mTotalX;
		double mTotalY;
		double mMoldX;
		double mMoldY;
		std::vector<int> mVecLeadNum;	           //Lead count
		std::vector<double> mVecLeadPitch;	       //Lead pitch
		std::vector<double> mVecLeadLength;          //Lead length
		std::vector<double> mVecLeadWidth;	       //Lead width
		std::vector<double> mVecCenterX;             //Lead-group center x
		std::vector<double> mVecCenterY;             //Lead-group center y
		std::vector<std::vector<int>> mVecCutParam;  // Missing leads

		// Matching parameters
		int mStepNum;
		std::vector<float> mSampleSteps;
		std::vector<int> mPyramidLevels;
		std::vector<float> mStepPixels;
		std::vector<float> mStepAngles;
		std::vector<int> mSobelSizes;
		std::vector<float> mMargins;
		float mAngleRange;  // Normal matching angle range

		// Shape template
		std::vector<std::vector<cv::Mat>> mLeadStepTemplates;
		std::vector<std::vector<cv::Mat>> mLeadRotTemplatesCoarse;
		std::vector<std::vector<cv::Mat>> mLeadRotTemplates;
		std::vector<std::vector<cv::Mat>> mLeadMultiScaleTemplates;

		// Result checking
		cv::Mat mPartImg;
		cv::Point mLeftTop;
		//std::vector<std::vector<cv::Mat>> mPinTemplates;
		cv::Point2f mSrcImgCtr;
		cv::Point2f mCropImgCtr;
		cv::Point2d mSclFac;
		cv::Mat mCropImg;
		cv::Mat mCropMagImg;
	};
}

#endif
