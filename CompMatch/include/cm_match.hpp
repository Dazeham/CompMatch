#pragma once
#ifndef CM_MATCH_HPP
#define CM_MATCH_HPP
#include <vector>
#include <opencv2/opencv.hpp>
#include "cm_error_code.hpp"
#include "cm_comp_data.hpp"
#define showTimeFlag false

namespace cm {
	class CTemplatePartGroup
	{
	public:
		CTemplatePartGroup() = delete;
		CTemplatePartGroup(float inScaleX, float inScaleY, float inBeginAngle = -30, float inEndAngle = 30, int inPyramidLevels = 2, float inSplItv = 1) : mScaleX(inScaleX), mScaleY(inScaleY), mBeginAngle(inBeginAngle), mEndAngle(inEndAngle), mPyramidLevels(inPyramidLevels), mSplItv(inSplItv), mFailNum(0) {};
		~CTemplatePartGroup() {};
        virtual AnswerType GenerateTemplate(std::shared_ptr<Component> inCompPtr) = 0;
        virtual AnswerType TemplateMatch(const cv::Mat& inSrcImg, cv::Point2f& outOffset, float& outAngle, float& outScore) = 0;
		virtual AnswerType SaveResult(const cv::Mat& inSrcImg, const cv::Point2f& inOffset, const float& inAngle, const float& inScore, const AnswerType& inAnswer, const double& inTime, const std::string& inPath) = 0;

		/* 形状模板匹配结构体 */
	public:
		struct matchPoint {
			float scoreG;
			float scoreM;
			float scoreV;
			float angle;
			float x;
			float y;
			matchPoint(float inScoreG = 0, float inScoreM = 0, float inScoreV = 0, float inAngle = 0, float inX = 0, float inY = 0) {
				scoreG = inScoreG;
				scoreM = inScoreM;
				scoreV = inScoreV;
				angle = inAngle;
				x = inX;
				y = inY;
			}
		};
		
		/* 形状模板匹配函数 */
	public:
		// 模板绘制
		AnswerType GetStepAngles(const cv::Size2d& inSize, const std::vector<int>& inPyramidLevels, std::vector<float>& outStepAngles);
		AnswerType GetTranslatedShapeTemplate(const std::vector<cv::Mat>& inSrcTpl, std::vector<cv::Mat>& outRotTpl, const cv::Point2f& inOffset);
		AnswerType GetRotatedShapeTemplate(const std::vector<cv::Mat>& inSrcTpl, std::vector<cv::Mat>& outRotTpl, const float& inAng);
		AnswerType GetRotatedShapeTemplates(const std::vector<cv::Mat>& inSrcTpl, std::vector<std::vector<cv::Mat>>& outRotTpls, const float& inBegAng, const float& inEndAng, const float& inStepAng);
		AnswerType GetMergedShapeTemplate(const std::vector<std::vector<cv::Mat>>& inSrcTpls, std::vector<cv::Mat>& outMerTpl);
		AnswerType GetPyramidShapeTemplate(const std::vector<cv::Mat>& inSrcTpl, std::vector<cv::Mat>& outPyrTpl, const int& inPyrLvl);
		AnswerType GetPyramidShapeTemplates(const std::vector<cv::Mat>& inSrcTpl, std::vector<std::vector<cv::Mat>>& outPyrTpls, const int& inPyrLvl);
		AnswerType GetSampleShapeTemplates(const std::vector<std::vector<cv::Mat>>& inTpls, std::vector<std::vector<cv::Mat>>& outTpls, const int& inMaxPtNum = 9999);
		AnswerType GetBaseSampleStep(const std::vector<cv::Mat>& inSrcTpl, float& outBaseSplStep, const int& inMaxPtNum = 9999);
		AnswerType GetStrictSampleShapeTemplates(const std::vector<std::vector<cv::Mat>>& inTpls, std::vector<std::vector<cv::Mat>>& outTpls, const int& inMaxPtNum = 9999);
		AnswerType GetShapeTemplateSize(const std::vector<cv::Mat>& inSrcTpl, cv::Size2f& outTplSize);
		AnswerType GetShapeDiagonalLength(const cv::Size2f& inRectSize, float& outDiaLen);
		AnswerType GetScaleShapeTemplate(std::vector<cv::Mat>& inSrcTpl, const cv::Point2d& inSclFac);
		AnswerType GetScaleShapeTemplates(std::vector<std::vector<cv::Mat>>& inSrcTpls, const cv::Point2d& inSclFac);
		// 结果检查
		AnswerType CheckShapeTemplates(const std::vector<cv::Mat>& inSrcTpl);
		AnswerType CheckShapeTemplatesSVG(const std::vector<cv::Mat>& inSrcTpl);
		AnswerType CheckMatchPoints(const cv::Mat& inSrcImg, const std::vector<matchPoint>& inMatchPts);
		AnswerType CheckMatchTemplate(const cv::Mat& inSrcImg, const std::vector<cv::Mat>& inSrcTpl, const cv::Point2f& inPos, const cv::Point2d& inSclFac = cv::Point2d(1, 1));
		AnswerType CheckMatchTemplateSVG(const cv::Mat& inSrcImg, const std::vector<cv::Mat>& inSrcTpl, const cv::Point2f& inOffset, const float& inAngle, const int& inPyrLvl, const int& inMinPyrLvl, const cv::Point2d& inSclFac);
		AnswerType CheckMatchTemplate(const cv::Mat& inSrcImg, const std::vector<cv::Mat>& inSrcTpl, const cv::Point2f& inOffset, const float& inAngle, const int& inPyrLvl, const int& inMinPyrLvl, const cv::Point2d& inSclFac = cv::Point2d(1, 1));
		cv::Mat GetCheckMatchTemplateImage(const cv::Mat& inSrcImg, const std::vector<cv::Mat>& inSrcTpl, const cv::Point2f& inOffset, const float& inAngle, const int& inPyrLvl, const int& inMinPyrLvl, const cv::Point2d& inSclFac = cv::Point2d(1, 1));
		cv::Mat GetCheckMatchTemplateErrorImage(const cv::Mat& inSrcImg);
		// 匹配
		AnswerType GetRegionRectImage(const cv::Mat& inSrcImg, cv::Mat& outResImg, const cv::Size2f& inPartSize, const cv::Point2d& inScale);
		bool GetConvexHull(const cv::Mat& inMagImg, const std::vector<std::vector<cv::Mat>>& inRotTpls, const cv::Size2f& inCompSize, std::vector<cv::Point>& outDetectRange);
		bool GetConvexHullX(const cv::Mat& inMagImg, const std::vector<std::vector<cv::Mat>>& inRotTpls, const cv::Size2f& inCompSize, std::vector<cv::Point>& outDetectRange);
		AnswerType GetDenoiseGradientAndMagnitudeImages(std::vector<cv::Mat>& inGradImgs, cv::Mat& inMagImg, const float& inMaxMag, const float& inMagTrs = 0.1);
		AnswerType MatchRotatedTemplatesCoarse(const cv::Point2f& inScale, const cv::Mat& inPartImg, const std::vector<std::vector<cv::Mat>>& inRotTpls, const cv::Size2f& inTotalSize, const cv::Size2f& inMoldSize, const int& inBegPixelX, const int& inEndPixelX, const int& inBegPixelY, const int& inEndPixelY, const int& inStepPixel, const float& inBegAng, const float& inStepAng, const int& inMaxPosNum, const int& inMaxNum, cv::Mat& outCrsMagImg, std::vector<cv::Point2f>& outMatchOffsets, std::vector<float>& outMatchAngles, std::vector<double>& outMatchScores, const int& inPyrLvl);
		AnswerType MatchRotatedTemplatesNormal(const cv::Size2f& inTotalSize, const cv::Mat& inPartImg, const std::vector<std::vector<cv::Mat>>& inRotTpls, const float& inBegAng, std::vector<cv::Point2f>& inOffsets, const int& inMargin, std::vector<float>& inAngles, const float& inAngleRange, const int& inStepPixel, const float& inStepAng, cv::Point2f& outOffset, float& outAngle, const int& inPyrLvl);
		AnswerType MatchRotatedTemplatesPrecise(const cv::Size2f& inTotalSize, const cv::Mat& inPartImg, const std::vector<std::vector<cv::Mat>>& inMultiScaleTpls, cv::Point2f& inOffset, const int& inMargin, float& inAngle, const float& inAngleRange, const float& inStepPixel, const float& inStepAng, cv::Point2f& outOffset, float& outAngle, double& outScore, cv::Mat& outCropImg, std::vector<cv::Mat>& outCropGradImgs, cv::Mat& outCropMagImg, cv::Point& outLeftTop, float& outMaxMagVal, const int& inPyrLvl, const cv::Point2d& inSclFac = cv::Point2d(1, 1));
		AnswerType PartDetect(const cv::Point2f& inScale, const cv::Mat& inPartImg, const std::vector<std::vector<cv::Mat>>& inStepTemplates, const std::vector<std::vector<cv::Mat>>& inRotTemplatesCoarse, const std::vector<std::vector<cv::Mat>>& inRotTemplates, const std::vector<std::vector<cv::Mat>>& inMultiScaleTemplates, const std::vector<int>& inPyramidLevels, const std::vector<float>& inStepPixels, const float& inBeginAngle, const float& inEndAngle, const float& inAngleRange, const std::vector<float>& inStepAngles, std::vector<int> inSobelSizes, const cv::Size2f& inTotalSize, const cv::Size2f& inMoldSize, const int& inStepNum, cv::Point2f& outOffset, float& outAngle, float& outScore, cv::Mat& outCrsMagImg, cv::Mat& outCropImg, std::vector<cv::Mat>& outCropGradImgs, cv::Mat& outCropMagImg, float& outMaxMagVal, cv::Point& outLeftTop, const cv::Point2d& inSclFac = cv::Point2d(1, 1));
		AnswerType PartResult(const cv::Point2d& inScale, const cv::Point2f& inOffset, const float& inAngle, const float& inScore, const float& inBegAng, const float& inEndAng);
		bool GetCroppedImage(const cv::Mat& inSrcImg, cv::Mat& outCropImg, const cv::Point2f& inOffset, const float& inAngle, const cv::Size2f& inSize);
		AnswerType GetMagnitudeErrorFlag(const cv::Mat& inMagBinImg, const std::vector<std::vector<cv::Mat>>& inConfTpls, const cv::Point2f& inOffset, const float& inAngle, const float& inMagErrT, std::vector<int>& outVecMagErrFlag);
		AnswerType GetPrecisePosition(const cv::Point2f& inSrcImgCtr, const cv::Point2d& inScaleFactor, const std::vector<cv::Mat>& inSrcTpl, const cv::Point2f& inScale, const cv::Size& inSrcImgSize, const cv::Mat& inCropImg, const cv::Mat& inCropMagImg, const std::vector<cv::Mat>& inCropGradImgs, const cv::Point& inLeftTop, const cv::Size2d& inTotalSize, cv::Point2f& inOffset, float& inAngle);

		/* 形状模板匹配参数 */
	public:
		double mScaleX;
		double mScaleY;
		float mEndAngle;
		float mBeginAngle;
		int mPyramidLevels;
		float mSplItv;
		int mFailNum;
	};
    typedef std::shared_ptr<CTemplatePartGroup> CTemplatePartGroupPtr;
}

#endif
