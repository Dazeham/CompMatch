#pragma once
#ifndef CM_MATCH_HPP
#define CM_MATCH_HPP
#include <opencv2/opencv.hpp>
#include "cm_error_code.hpp"
#include "cm_comp_data.hpp"
#include "cm_halcon.hpp"


namespace cm {
	class CTemplatePartGroup
	{
	public:
		CTemplatePartGroup() = delete;
		CTemplatePartGroup(float inScaleX, float inScaleY, float inBeginAngle = -30, float inEndAngle = 30, int inBorWidth = 5, HalconCpp::HTuple inPyramidLevels = 3) : mScaleX(inScaleX), mScaleY(inScaleY), mBeginAngle(inBeginAngle), mEndAngle(inEndAngle), mBorWidth(inBorWidth), mPyramidLevels(inPyramidLevels) {};
		~CTemplatePartGroup() {};
		virtual AnswerType GenerateTemplate(std::shared_ptr<Component> inCompPtr) = 0;
		AnswerType TemplateMatch(const HalconCpp::HObject& inSrcImg, cv::Point2f& outOffset, float& outAngle, float& outScore);

	public:
		AnswerType GetTranslatedShapeTemplate(const HalconCpp::HObject& inSrcTpl, HalconCpp::HObject& outRotTpl, const cv::Point2f& inOffset);
		AnswerType GetRotatedShapeTemplate(const HalconCpp::HObject& inSrcTpl, HalconCpp::HObject& outRotTpl, const cv::Point2f& inCtr, const float& inAng);
		AnswerType GetMergedShapeTemplate(const std::vector<HalconCpp::HObject>& inSrcTpls, HalconCpp::HObject& outMerTpl);

	public:
		double mScaleX;
		double mScaleY;
		float mEndAngle;
		float mBeginAngle;
		int mBorWidth;
		HalconCpp::HTuple mPyramidLevels;
		HalconCpp::HTuple mModelID;
	};
    typedef std::shared_ptr<CTemplatePartGroup> CTemplatePartGroupPtr;
}

#endif
