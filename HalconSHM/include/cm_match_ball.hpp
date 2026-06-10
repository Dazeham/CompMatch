#pragma once
#ifndef CM_MATCH_BALL_HPP
#define CM_MATCH_BALL_HPP
#include "cm_match.hpp"


namespace cm {
	// Builds and matches HALCON shape models for ball-grid components.
	class CTemplateShapeBall : public CTemplatePartGroup {
	public:
		CTemplateShapeBall() = delete;
		CTemplateShapeBall(float inScaleX, float inScaleY);
		~CTemplateShapeBall() {};
		// Build a ball-grid HALCON shape model.
		AnswerType GenerateTemplate(std::shared_ptr<Component> inCompPtr);

	private:
		// Draw one solder ball as a HALCON contour template.
		AnswerType GetBallTemplate(HalconCpp::HObject& outLeadTpl, const float& inRadius);
		// Combine all solder balls into one component template.
		AnswerType GetBallSourceTemplate(HalconCpp::HObject& outBallCompTpl, const float& inRadius);

	private:
		double mTotalX;
		double mTotalY;
		double mMoldX;
		double mMoldY;
		int mNumX;  // Ball count in X direction
		int mNumY;  // Ball count in Y direction
		double mAvgR;  // Radius
		double mPitchX;  // Ball pitch in X direction
		double mPitchY;  // Ball pitch in Y direction
		cv::Mat mLack;  // Missing balls

		cv::Size mTplSize;                           // Template size
		cv::Point2f mTplCtr;                         // Template center
	};
}

#endif
