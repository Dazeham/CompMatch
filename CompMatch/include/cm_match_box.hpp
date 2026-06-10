#pragma once
#ifndef CM_MATCH_BOX_HPP
#define CM_MATCH_BOX_HPP
#include "cm_match.hpp"


namespace cm {
	// Matches rectangular or box-like components with optional lead edges.
	class CTemplateShapeBoxType : public CTemplatePartGroup {
	public:
		CTemplateShapeBoxType() = delete;
		CTemplateShapeBoxType(float inScaleX, float inScaleY);
		~CTemplateShapeBoxType() {};
		// Build box templates from component dimensions and lead options.
		AnswerType GenerateTemplate(std::shared_ptr<Component> inCompPtr);
		// Match a box component and return offset, angle, and score.
		AnswerType TemplateMatch(const cv::Mat& inSrcImg, cv::Point2f& outOffset, float& outAngle, float& outScore);
		// Save a box match overlay as an SVG result.
		AnswerType SaveResult(const cv::Mat& inSrcImg, const cv::Point2f& inOffset, const float& inAngle, const float& inScore, const AnswerType& inAnswer, const double& inTime, const std::string& inPath);

		// Member functions
	private:
		// Template drawing
		// Compute the corner radius used by rounded rectangle templates.
		AnswerType GetRectCornerRadius(const cv::Size2f& inRectSize, const double& inScaleX, const double& inScaleY, float& outRad);
		// Draw a rounded rectangle contour template.
		AnswerType GetRectSourceTemplate(const cv::Size2f& inRectSize, const float& inCorRad, const std::vector<int>& inVecBoxSide, std::vector<cv::Mat>& outSrcTpl, const float& inSplStep = 1);
		// Draw an alternate rounded rectangle contour template.
		AnswerType GetRectSourceTemplateX(const cv::Size2f& inRectSize, const float& inCorRad, const std::vector<int>& inVecBoxSide, std::vector<cv::Mat>& outSrcTpl, const float& inSplStep = 1);
		// Draw a single rectangular lead template.
		AnswerType GetRectLeadTemplate(const cv::Size2f& inRectLeadSize, const float& inCordRad, std::vector<cv::Mat>& outLeadSize, const float& inSplStep);
		// Draw a rectangle template with lead-aware side geometry.
		AnswerType GetRectSourceTemplateR(const cv::Size2f& inRectSize, const std::vector<int>& inVecLeadNum, const std::vector<double>& inVecLeadLength, const std::vector<double>& inVecLeadWidth, std::vector<double>& inVecCenterX, std::vector<double> inVecCenterY, const float& inCorRad, const std::vector<int>& inVecBoxSide, std::vector<cv::Mat>& outSrcTpl, const float& inSplStep, const int& inPyrLvl);
		// Draw the sampled box template used by the main matcher.
		AnswerType GetRectSourceTemplateS(const cv::Size2f& inRectSize, const std::vector<int>& inVecLeadNum, const std::vector<double>& inVecLeadLength, const std::vector<double>& inVecLeadWidth, std::vector<double>& inVecCenterX, std::vector<double> inVecCenterY, const float& inCorRad, const std::vector<int>& inVecBoxSide, std::vector<cv::Mat>& outSrcTpl, const float& inSplStep, const int& inPyrLvl);
		// Compute the size tolerance for multi-scale box templates.
		AnswerType GetRectSizeTolerance(const cv::Size2f& inRectSize, const double& inScaleX, const double& inScaleY, float& outTol);

		// Extra matching
		// Refine a box pose using edge fitting in four directions.
		AnswerType GetPreciseRectPosition(const cv::Point2f& inScale, const cv::Size& inSrcImgSize, const cv::Mat& inCropImg, const cv::Mat& inCropMagImg, const std::vector<cv::Mat>& inCropGradImgs, const cv::Point& inLeftTop, const cv::Size2d& inTotalSize, cv::Point2f& inOffset, float& inAngle, const std::vector<int>& inVecBoxSide, const float& inLineModOffset, const float& inLineModAngle, const float& inLineUseRatio, const float& inLineRowBegRatio, const float& inLineMagTresh, const float& inLineDistTresh);

		// Member variables
	private:
		const std::map<double, int> mMapAngleToIdx = { {90, 0}, {0, 1}, {-90, 2}, {180, 3} };
		const std::map<int, double> mMapIdxToAngle = { {0, 90}, {1, 0}, {2, -90}, {3, 180} };

		// Component parameters
		cv::Point2d mScaleFactor;
		double mTotalX;
		double mTotalY;
		std::vector<int> mVecLeadNum;	            //Lead count
		std::vector<double> mVecLeadLength;         //Lead length
		std::vector<double> mVecLeadWidth;	        //Lead width
		std::vector<double> mVecCenterX;            //Lead-group center x
		std::vector<double> mVecCenterY;            //Lead-group center y

		// Recognition options
		std::vector<int> mVecBoxSide;               //Invalid sides

		// Matching parameters
		int mStepNum;
		std::vector<float> mSampleSteps;
		std::vector<int> mPyramidLevels;
		std::vector<float> mStepPixels;
		std::vector<float> mStepAngles;
		std::vector<int> mSobelSizes;
		std::vector<float> mMargins;
		float mAngleRange;                          // Normal matching angle range

		// Extra matching parameters
		float mLineModOffser;
		float mLineModAngle;
		float mLineUseRatio;
		float mLineRowBegRatio;
		float mLineMagTresh;
		float mLineDistTresh;

		// Shape template
		std::vector<std::vector<cv::Mat>> mRectStepTemplates;
		std::vector<std::vector<cv::Mat>> mRectRotTemplatesCoarse;
		std::vector<std::vector<cv::Mat>> mRectRotTemplates;
		std::vector<std::vector<cv::Mat>> mRectMultiScaleTemplates;
		std::vector<cv::Mat> mRectTemplatesRect;

		// Result checking
		cv::Mat mPartImg;
		cv::Point mLeftTop;
		cv::Point2f mSrcImgCtr;
		cv::Point2f mCropImgCtr;
		cv::Point2d mScaleFactorInverse;
		cv::Mat mCropImg;
		cv::Mat mCropMagImg;
	};
}

#endif
