#pragma once
#ifndef CM_MATCH_LEAD_HPP
#define CM_MATCH_LEAD_HPP
#include "cm_match.hpp"


namespace cm {
	// Builds and matches HALCON shape models for leaded components.
	class CTemplateShapeLead : public CTemplatePartGroup {
	public:
		CTemplateShapeLead() = delete;
		CTemplateShapeLead(float inScaleX, float inScaleY);
		~CTemplateShapeLead() {};
		// Build a leaded-component HALCON shape model.
		AnswerType GenerateTemplate(std::shared_ptr<Component> inCompPtr);

	private:
		// Draw one lead as a HALCON contour template.
		AnswerType GetLeadTemplate(const cv::Size2f& inLeadSize, HalconCpp::HObject& outLeadTpl);
		// Arrange one lead template into a lead-side group.
		AnswerType GetLeadGroupTemplate(const HalconCpp::HObject& inLeadTpl, HalconCpp::HObject& outLeadGroupTpl, const int& inLeadNum, const float& inLeadPitch, const std::vector<int>& inCutParam, const float& inAng, const cv::Point2f& inLeadCenter);
		// Combine all lead groups into one component template.
		AnswerType GetLeadSourceTemplate(HalconCpp::HObject& outLeadCompTpl, const std::vector<double>& inLeadWidths, const std::vector<double>& inLeadLengths, const std::vector<int>& inLeadNums, const std::vector<double>& inLeadPitches, const std::vector<std::vector<int>>& inCutParams, const std::vector<double>& inCenterXs, const std::vector<double>& inCenterYs);

	private:
		const std::map<double, int> mMapAngleToIdx = { {90, 0}, {0, 1}, {-90, 2}, {180, 3} };
		const std::map<int, double> mMapIdxToAngle = { {0, 90}, {1, 0}, {2, -90}, {3, 180} };

		double mTotalX;
		double mTotalY;
		double mMoldX;
		double mMoldY;
		std::vector<int> mVecLeadNum;	             //Lead count
		std::vector<double> mVecLeadPitch;	         //Lead pitch
		std::vector<double> mVecLeadLength;          //Lead length
		std::vector<double> mVecLeadWidth;	         //Lead width
		std::vector<double> mVecCenterX;             //Lead-group center x
		std::vector<double> mVecCenterY;             //Lead-group center y
		std::vector<std::vector<int>> mVecCutParam;  // Missing leads

		cv::Size mTplSize;                           // Template size
		cv::Point2f mTplCtr;                         // Template center
	};
}

#endif
