#pragma once
#ifndef CM_MATCH_BALL_HPP
#define CM_MATCH_BALL_HPP
#include "cm_match.hpp"
#include "cm_svg.hpp"


namespace cm {
	class CTemplateShapeBall : public CTemplatePartGroup {
	public:
		CTemplateShapeBall() = delete;
		CTemplateShapeBall(float inScaleX, float inScaleY);
		~CTemplateShapeBall() {};
		AnswerType GenerateTemplate(std::shared_ptr<Component> inCompPtr);
		AnswerType TemplateMatch(const cv::Mat& inSrcImg, cv::Point2f& outOffset, float& outAngle, float& outScore);
		AnswerType SaveResult(const cv::Mat& inSrcImg, const cv::Point2f& inOffset, const float& inAngle, const float& inScore, const AnswerType& inAnswer, const double& inTime, const std::string& inPath);

		// Member functions
	private:
		// SVG contour
		AnswerType GetBallItem(SVGItem& outLeadTpl, const float& inRadius);
		AnswerType GetBallSourceItem(std::vector<SVGItem>& outBallCompTpl, const float& inRadius);

		// Template drawing
		AnswerType GetBallTemplate(std::vector<cv::Mat>& outLeadTpl, const float& inRadius, const float& inSplStep);
		AnswerType GetBallTemplateFix(std::vector<cv::Mat>& outLeadTpl, const float& inRadius, const int& inSplNum);
		AnswerType GetBallSourceTemplate(std::vector<cv::Mat>& outBallCompTpl, const float& inRadius, const float& inSplStep, const int& inPyrLvl);
		AnswerType GetSingleBallSourceTemplate(std::vector<std::vector<cv::Mat>>& outBallCompTpl, const float& inRadius, const float& inSplStep, const int& inPyrLvl);

		// Member variables
	private:
		// Component parameters
		cv::Point2d mScaleFactor;
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
		std::vector<std::vector<cv::Mat>> mBallStepTemplates;
		std::vector<std::vector<cv::Mat>> mBallRotTemplatesCoarse;
		std::vector<std::vector<cv::Mat>> mBallRotTemplates;
		std::vector<std::vector<cv::Mat>> mBallMultiScaleTemplates;

		// Result checking
		cv::Mat mPartImg;
		cv::Point mLeftTop;
		//std::vector<std::vector<cv::Mat>> mBallTemplates;
		cv::Point2f mSrcImgCtr;
		cv::Point2f mCropImgCtr;
		cv::Point2d mSclFac;
		cv::Mat mCropImg;
		cv::Mat mCropMagImg;
	};
}

#endif
