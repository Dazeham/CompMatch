#pragma once
#ifndef CM_MATCH_BALL_HPP
#define CM_MATCH_BALL_HPP
#include "cm_match.hpp"


namespace cm {
	class CTemplateShapeBall : public CTemplatePartGroup {
	public:
		CTemplateShapeBall() = delete;
		CTemplateShapeBall(float inScaleX, float inScaleY);
		~CTemplateShapeBall() {};
		AnswerType GenerateTemplate(std::shared_ptr<Component> inCompPtr);

	private:
		AnswerType GetBallTemplate(HalconCpp::HObject& outLeadTpl, const float& inRadius);
		AnswerType GetBallSourceTemplate(HalconCpp::HObject& outBallCompTpl, const float& inRadius);

	private:
		double mTotalX;
		double mTotalY;
		double mMoldX;
		double mMoldY;
		int mNumX;  // X轴方向球数
		int mNumY;  // Y轴方向球数
		double mAvgR;  // 半径
		double mPitchX;  // X轴方向球间距
		double mPitchY;  // Y轴方向球间距
		cv::Mat mLack;  // 缺失球

		cv::Size mTplSize;                           // 模板尺寸
		cv::Point2f mTplCtr;                         // 模板中心
	};
}

#endif
