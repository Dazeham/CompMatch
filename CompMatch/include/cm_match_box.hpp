#pragma once
#ifndef CM_MATCH_BOX_HPP
#define CM_MATCH_BOX_HPP
#include "cm_match.hpp"


namespace cm {
	class CTemplateShapeBoxType : public CTemplatePartGroup {
	public:
		CTemplateShapeBoxType() = delete;
		CTemplateShapeBoxType(float inScaleX, float inScaleY);
		~CTemplateShapeBoxType() {};
		AnswerType GenerateTemplate(std::shared_ptr<Component> inCompPtr);
		AnswerType TemplateMatch(const cv::Mat& inSrcImg, cv::Point2f& outOffset, float& outAngle, float& outScore);
		AnswerType SaveResult(const cv::Mat& inSrcImg, const cv::Point2f& inOffset, const float& inAngle, const float& inScore, const AnswerType& inAnswer, const double& inTime, const std::string& inPath);

		// 成员函数
	private:
		// 模板绘制
		AnswerType GetRectCornerRadius(const cv::Size2f& inRectSize, const double& inScaleX, const double& inScaleY, float& outRad);
		AnswerType GetRectSourceTemplate(const cv::Size2f& inRectSize, const float& inCorRad, const std::vector<int>& inVecBoxSide, std::vector<cv::Mat>& outSrcTpl, const float& inSplStep = 1);
		AnswerType GetRectSourceTemplateX(const cv::Size2f& inRectSize, const float& inCorRad, const std::vector<int>& inVecBoxSide, std::vector<cv::Mat>& outSrcTpl, const float& inSplStep = 1);
		AnswerType GetRectLeadTemplate(const cv::Size2f& inRectLeadSize, const float& inCordRad, std::vector<cv::Mat>& outLeadSize, const float& inSplStep);
		AnswerType GetRectSourceTemplateR(const cv::Size2f& inRectSize, const std::vector<int>& inVecLeadNum, const std::vector<double>& inVecLeadLength, const std::vector<double>& inVecLeadWidth, std::vector<double>& inVecCenterX, std::vector<double> inVecCenterY, const float& inCorRad, const std::vector<int>& inVecBoxSide, std::vector<cv::Mat>& outSrcTpl, const float& inSplStep, const int& inPyrLvl);
		AnswerType GetRectSourceTemplateS(const cv::Size2f& inRectSize, const std::vector<int>& inVecLeadNum, const std::vector<double>& inVecLeadLength, const std::vector<double>& inVecLeadWidth, std::vector<double>& inVecCenterX, std::vector<double> inVecCenterY, const float& inCorRad, const std::vector<int>& inVecBoxSide, std::vector<cv::Mat>& outSrcTpl, const float& inSplStep, const int& inPyrLvl);
		AnswerType GetRectSizeTolerance(const cv::Size2f& inRectSize, const double& inScaleX, const double& inScaleY, float& outTol);

		// 额外匹配
		AnswerType GetPreciseRectPosition(const cv::Point2f& inScale, const cv::Size& inSrcImgSize, const cv::Mat& inCropImg, const cv::Mat& inCropMagImg, const std::vector<cv::Mat>& inCropGradImgs, const cv::Point& inLeftTop, const cv::Size2d& inTotalSize, cv::Point2f& inOffset, float& inAngle, const std::vector<int>& inVecBoxSide, const float& inLineModOffset, const float& inLineModAngle, const float& inLineUseRatio, const float& inLineRowBegRatio, const float& inLineMagTresh, const float& inLineDistTresh);

		// 成员变量
	private:
		const std::map<double, int> mMapAngleToIdx = { {90, 0}, {0, 1}, {-90, 2}, {180, 3} };
		const std::map<int, double> mMapIdxToAngle = { {0, 90}, {1, 0}, {2, -90}, {3, 180} };

		// 元件参数
		cv::Point2d mScaleFactor;
		double mTotalX;
		double mTotalY;
		std::vector<int> mVecLeadNum;	            //引脚数目
		std::vector<double> mVecLeadLength;         //引脚长度
		std::vector<double> mVecLeadWidth;	        //引脚宽度
		std::vector<double> mVecCenterX;            //引脚组中心x
		std::vector<double> mVecCenterY;            //引脚组中心y

		// 识别可选项
		std::vector<int> mVecBoxSide;               //无效边

		// 匹配参数
		int mStepNum;
		std::vector<float> mSampleSteps;
		std::vector<int> mPyramidLevels;
		std::vector<float> mStepPixels;
		std::vector<float> mStepAngles;
		std::vector<int> mSobelSizes;
		std::vector<float> mMargins;
		float mAngleRange;                          // 普通匹配角度范围

		// 额外匹配参数
		float mLineModOffser;
		float mLineModAngle;
		float mLineUseRatio;
		float mLineRowBegRatio;
		float mLineMagTresh;
		float mLineDistTresh;

		// 形状模板
		std::vector<std::vector<cv::Mat>> mRectStepTemplates;
		std::vector<std::vector<cv::Mat>> mRectRotTemplatesCoarse;
		std::vector<std::vector<cv::Mat>> mRectRotTemplates;
		std::vector<std::vector<cv::Mat>> mRectMultiScaleTemplates;
		std::vector<cv::Mat> mRectTemplatesRect;

		// 结果检查
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
