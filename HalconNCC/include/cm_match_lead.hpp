#pragma once
#ifndef CM_MATCH_LEAD_HPP
#define CM_MATCH_LEAD_HPP
#include "cm_match.hpp"


namespace cm {
	class CTemplateShapeLead : public CTemplatePartGroup {
	public:
		CTemplateShapeLead() = delete;
		CTemplateShapeLead(float inScaleX, float inScaleY);
		~CTemplateShapeLead() {};
		AnswerType GenerateTemplate(std::shared_ptr<Component> inCompPtr);

	private:
		AnswerType GetLeadTemplate(const cv::Size2f& inLeadSize, HalconCpp::HObject& outLeadTpl);
		AnswerType GetLeadGroupTemplate(const HalconCpp::HObject& inLeadTpl, HalconCpp::HObject& outLeadGroupTpl, const int& inLeadNum, const float& inLeadPitch, const std::vector<int>& inCutParam, const float& inAng, const cv::Point2f& inLeadCenter);
		AnswerType GetLeadSourceTemplate(HalconCpp::HObject& outLeadCompTpl, const std::vector<double>& inLeadWidths, const std::vector<double>& inLeadLengths, const std::vector<int>& inLeadNums, const std::vector<double>& inLeadPitches, const std::vector<std::vector<int>>& inCutParams, const std::vector<double>& inCenterXs, const std::vector<double>& inCenterYs);

	private:
		const std::map<double, int> mMapAngleToIdx = { {90, 0}, {0, 1}, {-90, 2}, {180, 3} };
		const std::map<int, double> mMapIdxToAngle = { {0, 90}, {1, 0}, {2, -90}, {3, 180} };

		double mTotalX;
		double mTotalY;
		double mMoldX;
		double mMoldY;
		std::vector<int> mVecLeadNum;
		std::vector<double> mVecLeadPitch;	      
		std::vector<double> mVecLeadLength;       
		std::vector<double> mVecLeadWidth;	     
		std::vector<double> mVecCenterX;           
		std::vector<double> mVecCenterY;           
		std::vector<std::vector<int>> mVecCutParam; 

		cv::Size mTplSize;                          
		cv::Point2f mTplCtr;                      
	};
}

#endif
