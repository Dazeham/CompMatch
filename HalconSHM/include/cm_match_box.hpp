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

	private:
		double mTotalX;
		double mTotalY;

		cv::Size mTplSize;                           // 模板尺寸
		cv::Point2f mTplCtr;                         // 模板中心
	};
}

#endif
