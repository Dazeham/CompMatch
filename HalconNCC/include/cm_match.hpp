#pragma once
#ifndef CM_MATCH_HPP
#define CM_MATCH_HPP
#include <opencv2/opencv.hpp>
#include "cm_error_code.hpp"
#include "cm_comp_data.hpp"
#include "cm_halcon.hpp"


namespace cm {
	// Base class for HALCON NCC component matchers.
	class CTemplatePartGroup
	{
	public:
		CTemplatePartGroup() = delete;
		CTemplatePartGroup(float inScaleX, float inScaleY, float inBeginAngle = -30, float inEndAngle = 30, int inBorWidth = 5, HalconCpp::HTuple inPyramidLevels = 3) : mScaleX(inScaleX), mScaleY(inScaleY), mBeginAngle(inBeginAngle), mEndAngle(inEndAngle), mBorWidth(inBorWidth), mPyramidLevels(inPyramidLevels) {};
		~CTemplatePartGroup() {};
		// Build the HALCON model from component metadata.
		virtual AnswerType GenerateTemplate(std::shared_ptr<Component> inCompPtr) = 0;
		// Match the HALCON model and return pose and score.
		AnswerType TemplateMatch(const HalconCpp::HObject& inSrcImg, cv::Point2f& outOffset, float& outAngle, float& outScore);

	public:
		// Translate a HALCON contour template.
		AnswerType GetTranslatedShapeTemplate(const HalconCpp::HObject& inSrcTpl, HalconCpp::HObject& outRotTpl, const cv::Point2f& inOffset);
		// Rotate a HALCON contour template around a center.
		AnswerType GetRotatedShapeTemplate(const HalconCpp::HObject& inSrcTpl, HalconCpp::HObject& outRotTpl, const cv::Point2f& inCtr, const float& inAng);
		// Merge HALCON contour templates into one object.
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
