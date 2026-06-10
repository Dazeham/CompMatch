#pragma once
#ifndef CM_MATCH_BOX_HPP
#define CM_MATCH_BOX_HPP
#include "cm_match.hpp"


namespace cm {
	// Builds and matches HALCON NCC models for box components.
	class CTemplateShapeBoxType : public CTemplatePartGroup {
	public:
		CTemplateShapeBoxType() = delete;
		CTemplateShapeBoxType(float inScaleX, float inScaleY);
		~CTemplateShapeBoxType() {};
		// Build a box-component HALCON NCC model.
		AnswerType GenerateTemplate(std::shared_ptr<Component> inCompPtr);

	private:
		double mTotalX;
		double mTotalY;

		cv::Size mTplSize; 
		cv::Point2f mTplCtr; 
	};
}

#endif
