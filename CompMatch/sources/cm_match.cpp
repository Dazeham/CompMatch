#include <ppl.h>
#include "cm_match.hpp"
#include "cm_svg.hpp"
#include "cm_intrinsics.hpp"
#include "cm_utils.hpp"


using namespace cm;

int findClosestIndex(const std::vector<float>& inAngList, const float& inAngCor) {
	int closestIdx = 0;
	float minDiff = std::abs(inAngList[0] - inAngCor);

	for (size_t i = 1; i < inAngList.size(); ++i) {
		float diff = std::abs(inAngList[i] - inAngCor);
		if (diff < minDiff) {
			minDiff = diff;
			closestIdx = i;
		}
	}

	return closestIdx;
}

AnswerType CTemplatePartGroup::GetStepAngles(const cv::Size2d& inSize, const std::vector<int>& inPyramidLevels, std::vector<float>& outStepAngles) {
	AnswerType answer = IMG_SUCCESS_ANS().SetErrCode();

	/* 1 */
	/*
	outStepAngles.clear();
	for (int i = 0; i < inPyramidLevels.size(); ++i) {
		outStepAngles.push_back(std::atan(1. / (std::max(inSize.width, inSize.height) * pow(0.5, inPyramidLevels[i]))) * 180. / CV_PI);
	}
	*/

	/* 2 */
	/*
	float tmpAngCor = std::atan(1. / (std::max(inSize.width, inSize.height) * pow(0.5, inPyramidLevels[0]))) * 180. / CV_PI;
	const std::vector<float> angList = { 6, 5, 3 };
	tmpAngCor = angList[findClosestIndex(angList, tmpAngCor)];

	const float tmpAngPre = 0.1;

	float tmpAngNor = std::pow(tmpAngCor / tmpAngPre, 0.5) * tmpAngPre;
	tmpAngNor = std::max(tmpAngCor / round(tmpAngCor / tmpAngNor), tmpAngPre);
	
	outStepAngles = { tmpAngCor, tmpAngNor, tmpAngPre };
	*/

	/* 3 */
	///*
	outStepAngles.clear();
	outStepAngles = std::vector<float>(inPyramidLevels.size(), 0);
	for (int i = 0; i < inPyramidLevels.size(); ++i) {
		outStepAngles[i] = std::atan(1. / (std::max(inSize.width, inSize.height) * pow(0.5, inPyramidLevels[i]))) * 180. / CV_PI;
		if (i == 0) {
			const std::vector<float> angList = { 6, 5, 3 };
			outStepAngles[i] = angList[findClosestIndex(angList, outStepAngles[i])];
		}
		else {
			outStepAngles[i] = std::max(outStepAngles[i - 1] / std::max(round(outStepAngles[i-1] / outStepAngles[i]), (float)2), (float)0.1);
		}
	}
	//*/

	return answer;
}

AnswerType CTemplatePartGroup::GetTranslatedShapeTemplate(const std::vector<cv::Mat>& inSrcTpl, std::vector<cv::Mat>& outRotTpl, const cv::Point2f& inOffset) {
	AnswerType answer = IMG_SUCCESS_ANS().SetErrCode();

	cv::Mat tTX = inSrcTpl[0] + inOffset.x;
	cv::Mat tTY = inSrcTpl[1] + inOffset.y;
	cv::Mat tTGX = inSrcTpl[2];
	cv::Mat tTGY = inSrcTpl[3];

	// 灰度模板
	cv::Mat tTVX, tTVY;
	if (inSrcTpl.size() == 6) {
		tTVX = inSrcTpl[4] + inOffset.x;
		tTVY = inSrcTpl[5] + inOffset.y;
	}

	if (inSrcTpl.size() == 6) {
		outRotTpl = { tTX, tTY, tTGX, tTGY, tTVX, tTVY };
	}
	else {
		outRotTpl = { tTX, tTY, tTGX, tTGY };
	}

	return answer;
}

AnswerType CTemplatePartGroup::GetRotatedShapeTemplate(const std::vector<cv::Mat>& inSrcTpl, std::vector<cv::Mat>& outRotTpl, const float& inAng) {
	AnswerType answer = IMG_SUCCESS_ANS().SetErrCode();

	const float RT = CV_PI / 180.;

	const float r = inAng * RT;
	const float sin = std::sin(r);
	const float cos = std::cos(r);

	cv::Mat tRX = cos * inSrcTpl[0] - sin * inSrcTpl[1];
	cv::Mat tRY = sin * inSrcTpl[0] + cos * inSrcTpl[1];
	cv::Mat tRGX = inSrcTpl[2] * cos - inSrcTpl[3] * sin;
	cv::Mat tRGY = inSrcTpl[3] * cos + inSrcTpl[2] * sin;

	// 灰度模板
	cv::Mat tRVX, tRVY;
	if (inSrcTpl.size() == 6) {
		tRVX = cos * inSrcTpl[4] - sin * inSrcTpl[5];
		tRVY = sin * inSrcTpl[4] + cos * inSrcTpl[5];
	}

	if (inSrcTpl.size() == 6) {
		outRotTpl = { tRX, tRY, tRGX, tRGY, tRVX, tRVY };
	}
	else {
		outRotTpl = { tRX, tRY, tRGX, tRGY };
	}

	return answer;
}

AnswerType CTemplatePartGroup::GetRotatedShapeTemplates(const std::vector<cv::Mat>& inSrcTpl, std::vector<std::vector<cv::Mat>>& outRotTpls, const float& inBegAng, const float& inEndAng, const float& inStepAng) {
	AnswerType answer = IMG_SUCCESS_ANS().SetErrCode();

	outRotTpls.clear();
	int angleNum = round((inEndAng - inBegAng) / inStepAng) + 1;
	float angle = 0;
	for (int angleNo = 0; angleNo < angleNum; ++angleNo) {
		angle = angleNo * inStepAng + inBegAng;
		std::vector<cv::Mat> tmp;
		GetRotatedShapeTemplate(inSrcTpl, tmp, angle);
		outRotTpls.push_back(tmp);
	}

	return answer;
}

AnswerType CTemplatePartGroup::GetMergedShapeTemplate(const std::vector<std::vector<cv::Mat>>& inSrcTpls, std::vector<cv::Mat>& outMerTpl) {
	AnswerType answer = IMG_SUCCESS_ANS().SetErrCode();

	if (inSrcTpls.size() == 0) {
		answer = IMG_PARAM_ERROR_ANS().SetErrCode(IMG_PARAM_ERROR_ANS::ANS2::TemplateNumberError);
		return answer;
	}
	bool grayFlag = false;
	for (int tplNo = 0; tplNo < inSrcTpls.size(); ++tplNo) {
		if (inSrcTpls[tplNo].size() == 6) {
			grayFlag = true;
		}
		else if (inSrcTpls[tplNo].size() == 4) {
			continue;
		}
		else if (inSrcTpls[tplNo].size() == 0) {
			continue;
		}
		else {
			answer = IMG_PARAM_ERROR_ANS().SetErrCode(IMG_PARAM_ERROR_ANS::ANS2::TemplateNumberError);
			return answer;
		}
	}

	int tplSize = 0;
	for (int tplNo = 0; tplNo < inSrcTpls.size(); ++tplNo) {
		if (inSrcTpls[tplNo].size() > 0) {
			tplSize += inSrcTpls[tplNo][0].cols;
		}
	}

	cv::Mat tX = cv::Mat(cv::Size(tplSize, 1), CV_32FC1);
	cv::Mat tY = cv::Mat(cv::Size(tplSize, 1), CV_32FC1);
	cv::Mat tGX = cv::Mat(cv::Size(tplSize, 1), CV_32FC1);
	cv::Mat tGY = cv::Mat(cv::Size(tplSize, 1), CV_32FC1);
	cv::Rect leadRegion;
	int begin = 0;
	for (int tplNo = 0; tplNo < inSrcTpls.size(); ++tplNo) {
		if (inSrcTpls[tplNo].size() > 0 && inSrcTpls[tplNo][0].cols > 0 && inSrcTpls[tplNo][0].rows > 0) {
			leadRegion = cv::Rect(begin, 0, inSrcTpls[tplNo][0].cols, 1);
			tX(leadRegion) = inSrcTpls[tplNo][0] + 0;
			tY(leadRegion) = inSrcTpls[tplNo][1] + 0;
			tGX(leadRegion) = inSrcTpls[tplNo][2] + 0;
			tGY(leadRegion) = inSrcTpls[tplNo][3] + 0;
			begin += inSrcTpls[tplNo][0].cols;
		}
	}

	// 灰度模板
	cv::Mat tVX, tVY;
	if (grayFlag) {
		int tplSizeV = 0;
		for (int tplNo = 0; tplNo < inSrcTpls.size(); ++tplNo) {
			if (inSrcTpls[tplNo].size() == 6) {
				tplSizeV += inSrcTpls[tplNo][4].cols;
			}
		}

		tVX = cv::Mat(cv::Size(tplSizeV, 1), CV_32FC1);
		tVY = cv::Mat(cv::Size(tplSizeV, 1), CV_32FC1);
		cv::Rect leadRegionV;
		int beginV = 0;
		for (int tplNo = 0; tplNo < inSrcTpls.size(); ++tplNo) {
			if (inSrcTpls[tplNo].size() == 6 && inSrcTpls[tplNo][4].cols > 0 && inSrcTpls[tplNo][4].rows > 0) {
				leadRegionV = cv::Rect(beginV, 0, inSrcTpls[tplNo][4].cols, 1);
				tVX(leadRegionV) = inSrcTpls[tplNo][4] + 0;
				tVY(leadRegionV) = inSrcTpls[tplNo][5] + 0;
				beginV += inSrcTpls[tplNo][4].cols;
			}
		}
	}

	if (grayFlag) {
		outMerTpl = { tX, tY, tGX, tGY, tVX, tVY };
	}
	else {
		outMerTpl = { tX, tY, tGX, tGY };
	}

	return answer;
}

AnswerType CTemplatePartGroup::GetPyramidShapeTemplate(const std::vector<cv::Mat>& inSrcTpl, std::vector<cv::Mat>& outPyrTpl, const int& inPyrLvl) {
	AnswerType answer = IMG_SUCCESS_ANS().SetErrCode();

	const int srcTplSize = inSrcTpl[0].cols;
	const int outTplSize = floor(srcTplSize * powf(0.5, inPyrLvl));

	cv::Mat tX = cv::Mat(cv::Size(outTplSize, 1), CV_32FC1);
	cv::Mat tY = cv::Mat(cv::Size(outTplSize, 1), CV_32FC1);
	cv::Mat tGX = cv::Mat(cv::Size(outTplSize, 1), CV_32FC1);
	cv::Mat tGY = cv::Mat(cv::Size(outTplSize, 1), CV_32FC1);
	float* pX = tX.ptr<float>();
	float* pY = tY.ptr<float>();
	float* pGX = tGX.ptr<float>();
	float* pGY = tGY.ptr<float>();
	const float* pXi = inSrcTpl[0].ptr<float>();
	const float* pYi = inSrcTpl[1].ptr<float>();
	const float* pGXi = inSrcTpl[2].ptr<float>();
	const float* pGYi = inSrcTpl[3].ptr<float>();
	for (int col = 0; col < outTplSize; ++col) {
		pX[col] = pXi[(int)pow(2, inPyrLvl) * col] * pow(0.5, inPyrLvl);
		pY[col] = pYi[(int)pow(2, inPyrLvl) * col] * pow(0.5, inPyrLvl);
		pGX[col] = pGX[(int)pow(2, inPyrLvl) * col];
		pGY[col] = pGY[(int)pow(2, inPyrLvl) * col];
	}


	cv::Mat tVX, tVY;
	if (inSrcTpl.size() == 6) {
		const int srcTplSizeV = inSrcTpl[0].cols;
		const int outTplSizeV = floor(srcTplSizeV * powf(0.5, inPyrLvl));
		tVX = cv::Mat(cv::Size(outTplSizeV, 1), CV_32FC1);
		tVY = cv::Mat(cv::Size(outTplSizeV, 1), CV_32FC1);
		float* pVX = tVX.ptr<float>();
		float* pVY = tVY.ptr<float>();
		const float* pGXi = inSrcTpl[4].ptr<float>();
		const float* pGYi = inSrcTpl[5].ptr<float>();
		for (int col = 0; col < outTplSizeV; ++col) {
			pVX[col] = pGXi[(int)pow(2, inPyrLvl) * col] * powf(0.5, inPyrLvl);
			pVY[col] = pGXi[(int)pow(2, inPyrLvl) * col] * powf(0.5, inPyrLvl);
		}
	}


	if (inSrcTpl.size() == 6) {
		outPyrTpl = { tX, tY, tGX, tGY, tVX, tVY };
	}
	else {
		outPyrTpl = { tX, tY, tGX, tGY };
	}

	return answer;
}

AnswerType CTemplatePartGroup::GetPyramidShapeTemplates(const std::vector<cv::Mat>& inSrcTpl, std::vector<std::vector<cv::Mat>>& outPyrTpls, const int& inPyrLvl) {
	AnswerType answer = IMG_SUCCESS_ANS().SetErrCode();

	outPyrTpls.clear();
	outPyrTpls.push_back(inSrcTpl);
	for (int pyrLvl = 1; pyrLvl <= inPyrLvl; ++pyrLvl) {
		std::vector<cv::Mat> tmpTpl;
		GetPyramidShapeTemplate(inSrcTpl, tmpTpl, pyrLvl);
		outPyrTpls.push_back(tmpTpl);
	}

	return answer;
}

AnswerType CTemplatePartGroup::GetSampleShapeTemplates(const std::vector<std::vector<cv::Mat>>& inTpls, std::vector<std::vector<cv::Mat>>& outTpls, const int& inMaxPtNum) {
	AnswerType answer = IMG_SUCCESS_ANS().SetErrCode();

	std::vector<std::vector<cv::Mat>> srcTpls = inTpls;
	if (srcTpls.size() != 0) {
		if (srcTpls[0].size() != 0) {
			if (srcTpls[0][0].cols >= inMaxPtNum) {
				const float splRat = float(srcTpls[0][0].cols - 1) / float(inMaxPtNum - 1);

				std::vector<int> splIdx;
				int oldIdx = -1;
				for (int i = 0; i < inMaxPtNum; ++i) {
					int tmpIdx = i * splRat;
					if (tmpIdx > (srcTpls[0][0].cols - 1)) {
						break;
					}
					if (tmpIdx == oldIdx) {
						continue;
					}
					splIdx.push_back(tmpIdx);
					oldIdx = tmpIdx;
				}

				outTpls.clear();
				for (int tplNo = 0; tplNo < srcTpls.size(); ++tplNo) {
					const int srcTplSize = srcTpls[tplNo][0].cols;

					int tplSize = 0;
					for (int i = 0; i < splIdx.size(); ++i) {
						if (splIdx[i] < srcTplSize) ++tplSize;
					}

					cv::Mat tX = cv::Mat(cv::Size(tplSize, 1), CV_32FC1);
					cv::Mat tY = cv::Mat(cv::Size(tplSize, 1), CV_32FC1);
					cv::Mat tGX = cv::Mat(cv::Size(tplSize, 1), CV_32FC1);
					cv::Mat tGY = cv::Mat(cv::Size(tplSize, 1), CV_32FC1);
					float* pX = tX.ptr<float>();
					float* pY = tY.ptr<float>();
					float* pGX = tGX.ptr<float>();
					float* pGY = tGY.ptr<float>();
					float* pXi = srcTpls[tplNo][0].ptr<float>();
					float* pYi = srcTpls[tplNo][1].ptr<float>();
					float* pGXi = srcTpls[tplNo][2].ptr<float>();
					float* pGYi = srcTpls[tplNo][3].ptr<float>();
					for (int i = 0; i < tplSize; ++i) {
						pX[i] = pXi[splIdx[i]];
						pY[i] = pYi[splIdx[i]];
						pGX[i] = pGXi[splIdx[i]];
						pGY[i] = pGXi[splIdx[i]];
					}

					outTpls.push_back({ tX, tY, tGX, tGY });
				}

				return answer;
			}
		}
	}

	outTpls = srcTpls;

	return answer;
}

AnswerType CTemplatePartGroup::GetBaseSampleStep(const std::vector<cv::Mat>& inSrcTpl, float& outBaseSplStep, const int& inMaxPtNum) {
	AnswerType answer = IMG_SUCCESS_ANS().SetErrCode();

	const float tplSize = inSrcTpl[0].cols;
	if (tplSize <= inMaxPtNum) outBaseSplStep = 1;
	else outBaseSplStep = tplSize / inMaxPtNum;

	return answer;
}

AnswerType CTemplatePartGroup::GetStrictSampleShapeTemplates(const std::vector<std::vector<cv::Mat>>& inTpls, std::vector<std::vector<cv::Mat>>& outTpls, const int& inMaxPtNum) {
	AnswerType answer = IMG_SUCCESS_ANS().SetErrCode();

	std::vector<std::vector<cv::Mat>> srcTpls(inTpls.begin(), inTpls.end());
	for (int i = 0; i < srcTpls.size(); ++i) {
		const int tplSize = srcTpls[i][0].cols;
		if (tplSize > inMaxPtNum) {
			//tmpTpl[j] = srcTpls[i][j](cv::Rect(0, 0, 1, inMaxPtNum)).clone();
			std::vector<cv::Mat> tmpTpl(4);
			for (int j = 0; j < 4; ++j) {
				cv::Mat tmpMat(cv::Size(inMaxPtNum, 1), CV_32FC1);
				float* pTmpMat = tmpMat.ptr<float>();
				int movNum = tplSize - inMaxPtNum;
				float movStep = tplSize / float(movNum);
				cv::Mat movFlag = cv::Mat::zeros(cv::Size(tplSize, 1), CV_8UC1);
				uchar* pMF = movFlag.ptr<uchar>();
				for (int no = 0; no < movNum; ++no) {
					pMF[int(no * movStep)] = 255;
				}
				int tmpCol = 0;
				float* pSrcTpl = srcTpls[i][j].ptr<float>();
				for (int col = 0; col < tplSize; ++col) {
					if (pMF[col] == 0) {
						pTmpMat[tmpCol] = pSrcTpl[col];
						tmpCol++;
					}
				}
				tmpTpl[j] = tmpMat;
			}
			srcTpls[i] = tmpTpl;
		}
	}
	outTpls = srcTpls;

	if (inTpls.size() == 6) {
		outTpls.push_back(inTpls[4]);
		outTpls.push_back(inTpls[5]);
	}

	return answer;
}

AnswerType CTemplatePartGroup::GetShapeTemplateSize(const std::vector<cv::Mat>& inSrcTpl, cv::Size2f& outTplSize) {
	AnswerType answer = IMG_SUCCESS_ANS().SetErrCode();

	double minX, maxX, minY, maxY;
	cv::minMaxLoc(inSrcTpl[0], &minX, &maxX);
	cv::minMaxLoc(inSrcTpl[1], &minY, &maxY);

	outTplSize = cv::Size2f(abs(maxX - minX), abs(maxY - minY));

	return answer;
}

AnswerType CTemplatePartGroup::GetShapeDiagonalLength(const cv::Size2f& inRectSize, float& outDiaLen) {
	AnswerType answer = IMG_SUCCESS_ANS().SetErrCode();

	outDiaLen = std::sqrtf(std::powf(inRectSize.width, 2) + std::powf(inRectSize.height, 2));

	return answer;
}

AnswerType CTemplatePartGroup::GetScaleShapeTemplate(std::vector<cv::Mat>& inSrcTpl, const cv::Point2d& inSclFac) {
	AnswerType answer = IMG_SUCCESS_ANS().SetErrCode();

	if (inSclFac.x != 1) {
		inSrcTpl[0] *= inSclFac.x;
	}
	if (inSclFac.y != 1) {
		inSrcTpl[1] *= inSclFac.y;
	}
	if (inSclFac.x != 1 || inSclFac.y != 1) {
		cv::Mat sqrtMat(inSrcTpl[2].size(), CV_32FC1);
		cv::sqrt(inSrcTpl[2].mul(inSrcTpl[2]) * pow(inSclFac.y, 2) + inSrcTpl[3].mul(inSrcTpl[3]) * pow(inSclFac.x, 2), sqrtMat);
		inSrcTpl[2] = inSrcTpl[2] * inSclFac.y / sqrtMat;
		inSrcTpl[3] = inSrcTpl[3] * inSclFac.x / sqrtMat;
	}

	// 灰度模板
	if (inSrcTpl.size() == 6) {
		inSrcTpl[4] *= inSclFac.x;
		inSrcTpl[5] *= inSclFac.y;
	}

	return answer;
}

AnswerType CTemplatePartGroup::GetScaleShapeTemplates(std::vector<std::vector<cv::Mat>>& inSrcTpls, const cv::Point2d& inSclFac) {
	AnswerType answer = IMG_SUCCESS_ANS().SetErrCode();

	for (int tplNo = 0; tplNo < inSrcTpls.size(); ++tplNo) {
		GetScaleShapeTemplate(inSrcTpls[tplNo], inSclFac);
	}

	return answer;
}

// 结果检查
AnswerType CTemplatePartGroup::CheckShapeTemplates(const std::vector<cv::Mat>& inSrcTpl) {
	AnswerType answer = IMG_SUCCESS_ANS().SetErrCode();

	cv::Mat tX, tY, tGX, tGY;
	tX = inSrcTpl[0];
	tY = inSrcTpl[1];
	tGX = inSrcTpl[2];
	tGY = inSrcTpl[3];

	double minValX, maxValX, minValY, maxValY;
	cv::minMaxLoc(tX, &minValX, &maxValX);
	cv::minMaxLoc(tY, &minValY, &maxValY);

	cv::Size imgSize(ceilf(2 * std::max(abs(minValX), abs(maxValX))) + 4, ceilf(2 * std::max(abs(minValY), abs(maxValY))) + 4);
	cv::Point2f imgOffset(0.5 * (imgSize.width - 1), 0.5 * (imgSize.height - 1));

	cv::Mat tImg = cv::Mat::zeros(imgSize, CV_32FC1);
	cv::Mat tGXImg = cv::Mat::zeros(imgSize, CV_32FC1);
	cv::Mat tGYImg = cv::Mat::zeros(imgSize, CV_32FC1);
	for (int i = 0; i < tX.cols; i++) {
		int row = std::min(std::max(cvRound(tY.ptr<float>()[i] + imgOffset.y), 0), imgSize.height - 1);
		int col = std::min(std::max(cvRound(tX.ptr<float>()[i] + imgOffset.x), 0), imgSize.width - 1);

		tImg.ptr<float>(row)[col] = 255;
		tGXImg.ptr<float>(row)[col] = tGX.ptr<float>()[i];
		tGYImg.ptr<float>(row)[col] = tGY.ptr<float>()[i];
	}

	// 灰度模板
	cv::Mat tVImg;
	if (inSrcTpl.size() == 6) {
		cv::Mat tVX = inSrcTpl[4];
		cv::Mat tVY = inSrcTpl[5];
		tVImg = cv::Mat::zeros(imgSize, CV_32FC1);
		for (int i = 0; i < tVX.cols; ++i) {
			int row = std::min(std::max(cvRound(tVY.ptr<float>()[i] + imgOffset.y), 0), imgSize.height - 1);
			int col = std::min(std::max(cvRound(tVX.ptr<float>()[i] + imgOffset.x), 0), imgSize.width - 1);
			tVImg.ptr<float>(row)[col] = 255;
		}
	}

	return answer;
}

AnswerType CTemplatePartGroup::CheckShapeTemplatesSVG(const std::vector<cv::Mat>& inSrcTpl) {
	AnswerType answer = IMG_SUCCESS_ANS().SetErrCode();

	cv::Mat tX, tY, tGX, tGY;
	tX = inSrcTpl[0];
	tY = inSrcTpl[1];
	tGX = inSrcTpl[2];
	tGY = inSrcTpl[3];

	double minValX, maxValX, minValY, maxValY;
	cv::minMaxLoc(tX, &minValX, &maxValX);
	cv::minMaxLoc(tY, &minValY, &maxValY);

	cv::Size imgSize(ceilf(2 * std::max(abs(minValX), abs(maxValX))) + 4, ceilf(2 * std::max(abs(minValY), abs(maxValY))) + 4);
	cv::Point2f imgOffset(0.5 * (imgSize.width - 1), 0.5 * (imgSize.height - 1));

	cv::Mat tImg = cv::Mat::zeros(imgSize, CV_32FC1);
	cv::Mat tGXImg = cv::Mat::zeros(imgSize, CV_32FC1);
	cv::Mat tGYImg = cv::Mat::zeros(imgSize, CV_32FC1);
	for (int i = 0; i < tX.cols; i++) {
		int row = std::min(std::max(cvRound(tY.ptr<float>()[i] + imgOffset.y), 0), imgSize.height - 1);
		int col = std::min(std::max(cvRound(tX.ptr<float>()[i] + imgOffset.x), 0), imgSize.width - 1);

		tImg.ptr<float>(row)[col] = 255;
		tGXImg.ptr<float>(row)[col] = tGX.ptr<float>()[i];
		tGYImg.ptr<float>(row)[col] = tGY.ptr<float>()[i];
	}


	// 保存SVG
	SVGTool st("output2.svg", imgSize.width, imgSize.height);
	for (int i = 0; i < tX.cols; i++) {
		float row = std::min(std::max(tY.ptr<float>()[i] + imgOffset.y, float(0.)), float(imgSize.height - 1));
		float col = std::min(std::max(tX.ptr<float>()[i] + imgOffset.x, float(0.)), float(imgSize.width - 1));

		cv::Point2f centerPt(col, row);
		float radius = 1.5f;
		std::string color = "#0000FF"; // 绿色

		st.drawCircle(centerPt, radius, color);
	}
	st.close();


	// 灰度模板
	cv::Mat tVImg;
	if (inSrcTpl.size() == 6) {
		cv::Mat tVX = inSrcTpl[4];
		cv::Mat tVY = inSrcTpl[5];
		tVImg = cv::Mat::zeros(imgSize, CV_32FC1);
		for (int i = 0; i < tVX.cols; ++i) {
			int row = std::min(std::max(cvRound(tVY.ptr<float>()[i] + imgOffset.y), 0), imgSize.height - 1);
			int col = std::min(std::max(cvRound(tVX.ptr<float>()[i] + imgOffset.x), 0), imgSize.width - 1);
			tVImg.ptr<float>(row)[col] = 255;
		}
	}

	return answer;
}

AnswerType CTemplatePartGroup::CheckMatchPoints(const cv::Mat& inSrcImg, const std::vector<matchPoint>& inMatchPts) {
	AnswerType answer = IMG_SUCCESS_ANS().SetErrCode();

	cv::Mat matchImg;
	cv::cvtColor(inSrcImg, matchImg, cv::COLOR_GRAY2BGR);

	for (int ptNo = 0; ptNo < inMatchPts.size(); ++ptNo) {
		cv::circle(matchImg, cv::Point2f(inMatchPts[ptNo].x, inMatchPts[ptNo].y), 0, cv::Scalar(1, 0, 0), -1);
	}

	return answer;
}

AnswerType CTemplatePartGroup::CheckMatchTemplate(const cv::Mat& inSrcImg, const std::vector<cv::Mat>& inSrcTpl, const cv::Point2f& inPos, const cv::Point2d& inSclFac) {
	AnswerType answer = IMG_SUCCESS_ANS().SetErrCode();

	cv::Mat showImg = inSrcImg.clone();
	if (showImg.depth() != CV_32F) {
		showImg.convertTo(showImg, CV_32F);
	}
	if (showImg.channels() != 3) {
		cv::Mat c1 = showImg.clone();
		cv::Mat c2 = showImg.clone();
		cv::Mat c3 = showImg.clone();
		std::vector<cv::Mat> cs = { c1, c2, c3 };
		cv::merge(cs, showImg);
	}

	std::vector<cv::Mat> rotTpl;
	GetRotatedShapeTemplate(inSrcTpl, rotTpl, 0);
	GetScaleShapeTemplate(rotTpl, inSclFac);

	for (int i = 0; i < rotTpl[0].cols; i++) {
		int row = cvRound(rotTpl[1].ptr<float>()[i] + inPos.y);
		int col = cvRound(rotTpl[0].ptr<float>()[i] + inPos.x);

		if (row < 0 || row >= showImg.rows || col < 0 || col >= showImg.cols) {
			continue;
		}

		cv::Vec3f& pixel = showImg.at<cv::Vec3f>(row, col);
		pixel[0] = 255;
		pixel[1] = 0;
		pixel[2] = 0;
	}

	// 灰度模板
	if (inSrcTpl.size() == 6) {
		for (int i = 0; i < rotTpl[4].cols; i++) {
			int row = cvRound(rotTpl[5].ptr<float>()[i] + inPos.y);
			int col = cvRound(rotTpl[4].ptr<float>()[i] + inPos.x);

			if (row < 0 || row >= showImg.rows || col < 0 || col >= showImg.cols) {
				continue;
			}

			cv::Vec3f& pixel = showImg.at<cv::Vec3f>(row, col);
			pixel[0] = 0;
			pixel[1] = 255;
			pixel[2] = 0;
		}
	}

	return answer;
}

AnswerType CTemplatePartGroup::CheckMatchTemplate(const cv::Mat& inSrcImg, const std::vector<cv::Mat>& inSrcTpl, const cv::Point2f& inOffset, const float& inAngle, const int& inPyrLvl, const int& inMinPyrLvl, const cv::Point2d& inSclFac) {
	AnswerType answer = IMG_SUCCESS_ANS().SetErrCode();

	cv::Mat showImg = inSrcImg.clone();
	if (showImg.depth() != CV_32F) {
		showImg.convertTo(showImg, CV_32F);
	}
	if (showImg.channels() != 3) {
		cv::Mat c1 = showImg.clone();
		cv::Mat c2 = showImg.clone();
		cv::Mat c3 = showImg.clone();
		std::vector<cv::Mat> cs = { c1, c2, c3 };
		cv::merge(cs, showImg);
	}

	std::vector<cv::Mat> rotTpl;
	GetRotatedShapeTemplate(inSrcTpl, rotTpl, inAngle);
	GetScaleShapeTemplate(rotTpl, inSclFac);

	cv::Point2f imgCenter((inSrcImg.cols - 1) * 0.5, (inSrcImg.rows - 1) * 0.5);
	cv::Point2f offset(imgCenter.x + inOffset.x * powf(0.5, inPyrLvl - inMinPyrLvl), imgCenter.y + inOffset.y * powf(0.5, inPyrLvl - inMinPyrLvl));

	for (int i = 0; i < rotTpl[0].cols; i++) {
		int row = cvRound(rotTpl[1].ptr<float>()[i] + offset.y);
		int col = cvRound(rotTpl[0].ptr<float>()[i] + offset.x);

		if (row < 0 || row >= showImg.rows || col < 0 || col >= showImg.cols) {
			continue;
		}

		cv::Vec3f& pixel = showImg.at<cv::Vec3f>(row, col);
		pixel[0] = 255;
		pixel[1] = 0;
		pixel[2] = 0;
	}

	// 灰度模板
	if (inSrcTpl.size() == 6) {
		for (int i = 0; i < rotTpl[4].cols; i++) {
			int row = cvRound(rotTpl[5].ptr<float>()[i] + offset.y);
			int col = cvRound(rotTpl[4].ptr<float>()[i] + offset.x);

			if (row < 0 || row >= showImg.rows || col < 0 || col >= showImg.cols) {
				continue;
			}

			cv::Vec3f& pixel = showImg.at<cv::Vec3f>(row, col);
			pixel[0] = 0;
			pixel[1] = 255;
			pixel[2] = 0;
		}
	}

	return answer;
}

AnswerType CTemplatePartGroup::CheckMatchTemplateSVG(const cv::Mat& inSrcImg, const std::vector<cv::Mat>& inSrcTpl, const cv::Point2f& inOffset, const float& inAngle, const int& inPyrLvl, const int& inMinPyrLvl, const cv::Point2d& inSclFac) {
	AnswerType answer = IMG_SUCCESS_ANS().SetErrCode();

	cv::Mat showImg = inSrcImg.clone();
	if (showImg.depth() != CV_32F) {
		showImg.convertTo(showImg, CV_32F);
	}
	if (showImg.channels() != 3) {
		cv::Mat c1 = showImg.clone();
		cv::Mat c2 = showImg.clone();
		cv::Mat c3 = showImg.clone();
		std::vector<cv::Mat> cs = { c1, c2, c3 };
		cv::merge(cs, showImg);
	}

	std::vector<cv::Mat> rotTpl;
	GetRotatedShapeTemplate(inSrcTpl, rotTpl, inAngle);
	GetScaleShapeTemplate(rotTpl, inSclFac);

	cv::Point2f imgCenter((inSrcImg.cols - 1) * 0.5, (inSrcImg.rows - 1) * 0.5);
	cv::Point2f offset(imgCenter.x + inOffset.x * powf(0.5, inPyrLvl - inMinPyrLvl), imgCenter.y + inOffset.y * powf(0.5, inPyrLvl - inMinPyrLvl));

	for (int i = 0; i < rotTpl[0].cols; i++) {
		int row = cvRound(rotTpl[1].ptr<float>()[i] + offset.y);
		int col = cvRound(rotTpl[0].ptr<float>()[i] + offset.x);

		if (row < 0 || row >= showImg.rows || col < 0 || col >= showImg.cols) {
			continue;
		}

		cv::Vec3f& pixel = showImg.at<cv::Vec3f>(row, col);
		pixel[0] = 255;
		pixel[1] = 0;
		pixel[2] = 0;
	}

	// 灰度模板
	if (inSrcTpl.size() == 6) {
		for (int i = 0; i < rotTpl[4].cols; i++) {
			int row = cvRound(rotTpl[5].ptr<float>()[i] + offset.y);
			int col = cvRound(rotTpl[4].ptr<float>()[i] + offset.x);

			if (row < 0 || row >= showImg.rows || col < 0 || col >= showImg.cols) {
				continue;
			}

			cv::Vec3f& pixel = showImg.at<cv::Vec3f>(row, col);
			pixel[0] = 0;
			pixel[1] = 255;
			pixel[2] = 0;
		}
	}

	// 保存SVG
	SVGTool st("output.svg", inSrcImg);
	for (int i = 0; i < rotTpl[0].cols; i++) {
		float row = rotTpl[1].ptr<float>()[i] + offset.y;
		float col = rotTpl[0].ptr<float>()[i] + offset.x;

		if (row < 0 || row >= showImg.rows || col < 0 || col >= showImg.cols) {
			continue;
		}


		cv::Point2f centerPt(col, row);
		float radius = 1.5f;
		std::string color = "#0000FF"; // 绿色

		st.drawCircle(centerPt, radius, color);
	}
	st.close();

	return answer;
}

cv::Mat CTemplatePartGroup::GetCheckMatchTemplateImage(const cv::Mat& inSrcImg, const std::vector<cv::Mat>& inSrcTpl, const cv::Point2f& inOffset, const float& inAngle, const int& inPyrLvl, const int& inMinPyrLvl, const cv::Point2d& inSclFac) {
	cv::Mat showImg = inSrcImg.clone();
	if (showImg.depth() != CV_32F) {
		showImg.convertTo(showImg, CV_32F);
	}
	if (showImg.channels() != 3) {
		cv::Mat c1 = showImg.clone();
		cv::Mat c2 = showImg.clone();
		cv::Mat c3 = showImg.clone();
		std::vector<cv::Mat> cs = { c1, c2, c3 };
		cv::merge(cs, showImg);
	}

	std::vector<cv::Mat> rotTpl;
	GetRotatedShapeTemplate(inSrcTpl, rotTpl, inAngle);
	GetScaleShapeTemplate(rotTpl, inSclFac);

	cv::Point2f imgCenter((inSrcImg.cols - 1) * 0.5, (inSrcImg.rows - 1) * 0.5);
	cv::Point2f offset(imgCenter.x + inOffset.x * powf(0.5, inPyrLvl - inMinPyrLvl), imgCenter.y + inOffset.y * powf(0.5, inPyrLvl - inMinPyrLvl));

	for (int i = 0; i < rotTpl[0].cols; i++) {
		int row = cvRound(rotTpl[1].ptr<float>()[i] + offset.y);
		int col = cvRound(rotTpl[0].ptr<float>()[i] + offset.x);

		if (row < 0 || row >= showImg.rows || col < 0 || col >= showImg.cols) {
			continue;
		}

		cv::Vec3f& pixel = showImg.at<cv::Vec3f>(row, col);
		pixel[0] = 255;
		pixel[1] = 0;
		pixel[2] = 0;
	}

	// 灰度模板
	if (inSrcTpl.size() == 6) {
		for (int i = 0; i < rotTpl[4].cols; i++) {
			int row = cvRound(rotTpl[5].ptr<float>()[i] + offset.y);
			int col = cvRound(rotTpl[4].ptr<float>()[i] + offset.x);

			if (row < 0 || row >= showImg.rows || col < 0 || col >= showImg.cols) {
				continue;
			}

			cv::Vec3f& pixel = showImg.at<cv::Vec3f>(row, col);
			pixel[0] = 0;
			pixel[1] = 255;
			pixel[2] = 0;
		}
	}

	cv::Mat wrtImg;
	showImg.convertTo(wrtImg, CV_8UC3);
	return wrtImg;
}

cv::Mat CTemplatePartGroup::GetCheckMatchTemplateErrorImage(const cv::Mat& inSrcImg) {
	cv::Mat showImg = inSrcImg.clone();
	if (showImg.depth() != CV_8U) {
		showImg.convertTo(showImg, CV_8U);
	}
	if (showImg.channels() != 3) {
		cv::Mat c1 = showImg.clone();
		cv::Mat c2 = showImg.clone();
		cv::Mat c3 = showImg.clone();
		std::vector<cv::Mat> cs = { c1, c2, c3 };
		cv::merge(cs, showImg);
	}
	line(showImg, cv::Point(0, 0), cv::Point(inSrcImg.cols - 1, inSrcImg.rows - 1), cv::Scalar(0, 0, 255), 2, cv::LINE_AA);
	line(showImg, cv::Point(0, inSrcImg.rows - 1), cv::Point(inSrcImg.cols - 1, 0), cv::Scalar(0, 0, 255), 2, cv::LINE_AA);
	return showImg;
}

// 匹配
AnswerType CTemplatePartGroup::GetRegionRectImage(const cv::Mat& inSrcImg, cv::Mat& outResImg, const cv::Size2f& inPartSize, const cv::Point2d& inScale) {
	AnswerType answer = IMG_SUCCESS_ANS().SetErrCode();

	cv::Size imgSize = inSrcImg.size();
	cv::Rect partRegionRect;

	int nozzleOffset = 2. / (inScale.x + inScale.y) * 2.;
	nozzleOffset = (nozzleOffset / 2) * 2;

	int partDiaLen = floor(std::sqrtf(std::powf(inPartSize.width, 2) + std::powf(inPartSize.height, 2)));
	int partRegionWidth = imgSize.width % 2 == 1 ? 2 * partDiaLen - 1 : 2 * partDiaLen;
	int partRegionHeight = imgSize.height % 2 == 1 ? 2 * partDiaLen - 1 : 2 * partDiaLen;
	partRegionWidth += nozzleOffset;
	partRegionHeight += nozzleOffset;
	int borderWidth = ceil((imgSize.width - partRegionWidth) * 0.5);
	int borderHeight = ceil((imgSize.height - partRegionHeight) * 0.5);

	((borderWidth < 1) || (partRegionWidth > imgSize.width)) ? (partRegionRect.x = 0, partRegionRect.width = imgSize.width) : (partRegionRect.x = borderWidth, partRegionRect.width = imgSize.width - 2 * borderWidth);
	((borderHeight < 1) || (partRegionHeight > imgSize.height)) ? (partRegionRect.y = 0, partRegionRect.height = imgSize.height) : (partRegionRect.y = borderHeight, partRegionRect.height = imgSize.height - 2 * borderHeight);

	if (partRegionRect.width == imgSize.width && partRegionRect.height == imgSize.height) {
		outResImg = inSrcImg;
	}
	else {
		outResImg = inSrcImg(partRegionRect).clone();
	}

	return answer;
}

bool CTemplatePartGroup::GetConvexHull(const cv::Mat& inMagImg, const std::vector<std::vector<cv::Mat>>& inRotTpls, const cv::Size2f& inCompSize, std::vector<cv::Point>& outDetectRange) {
	AnswerType answer = IMG_SUCCESS_ANS().SetErrCode();

	// 计算凸包边界
	std::vector<cv::Point> nonZeroPts;
	cv::findNonZero(inMagImg > 0.3, nonZeroPts);
	if (nonZeroPts.size() < 3) {
		answer = IMG_PARAM_ERROR_ANS().SetErrCode(IMG_PARAM_ERROR_ANS::ANS2::ConvexHullPointsNumberError);
		return false;
	}

	cv::Rect roi = cv::boundingRect(nonZeroPts);
	int minX = roi.x, minY = roi.y, maxX = roi.x + roi.width, maxY = roi.y + roi.height;

	// 计算缩小半径
	std::vector<double> vecTplMinX(inRotTpls.size(), 0);
	std::vector<double> vecTplMinY(inRotTpls.size(), 0);
	std::vector<double> vecTplMaxX(inRotTpls.size(), 0);
	std::vector<double> vecTplMaxY(inRotTpls.size(), 0);
	for (int tplNo = 0; tplNo < inRotTpls.size(); ++tplNo) {
		cv::minMaxLoc(inRotTpls[tplNo][0], &vecTplMinX[tplNo], &vecTplMaxX[tplNo]);
		cv::minMaxLoc(inRotTpls[tplNo][1], &vecTplMinY[tplNo], &vecTplMaxY[tplNo]);
	}

	if (vecTplMaxX.size() == 0 || vecTplMinX.size() == 0 || vecTplMaxY.size() == 0 || vecTplMinY.size() == 0) {
		return false;
	}
	const double ratio = 0.94;
	const double tplWidth = (*std::min_element(vecTplMaxX.begin(), vecTplMaxX.end()) - *std::max_element(vecTplMinX.begin(), vecTplMinX.end())) * ratio;
	const double tplHeight = (*std::min_element(vecTplMaxY.begin(), vecTplMaxY.end()) - *std::max_element(vecTplMinY.begin(), vecTplMinY.end())) * ratio;

	// 最终匹配区域
	if (maxX - minX - tplWidth <= 0 || maxY - minY - tplHeight <= 0 || tplWidth <= 0 || tplHeight <= 0) {
		return false;
	}
	else {
		minX += tplWidth * 0.5 - 1;
		maxX -= tplWidth * 0.5 - 1;
		minY += tplHeight * 0.5 - 1;
		maxY -= tplHeight * 0.5 - 1;
	}

	outDetectRange.clear();
	for (int x = minX; x <= maxX; ++x) {
		for (int y = minY; y <= maxY; ++y) {
			outDetectRange.push_back(cv::Point(x, y));
		}
	}


#ifdef _DEBUG
	cv::Mat showImg;
	cv::cvtColor(inMagImg, showImg, cv::COLOR_GRAY2BGR);
	cv::drawContours(showImg, std::vector<std::vector<cv::Point>>{{cv::Point(minX, minY), cv::Point(maxX, minY), cv::Point(maxX, maxY), cv::Point(minX, maxY)}}, -1, cv::Scalar(1, 0, 0), 1);  // 显示凸包
	cv::rectangle(showImg, cv::Point(minX, minY), cv::Point(maxX, maxY), cv::Scalar(0, 0, 1));  // 显示位置约束
#endif

	return true;
}

inline void GetContinuousRegions(const cv::Mat& inSrcImg, const cv::Point& inLeftTop, const double& inTplWid, const double& inTplHei, std::vector<cv::Point>& outMatPos) {
	outMatPos.clear();
	if (inSrcImg.empty() || inSrcImg.type() != CV_8UC1) return;

	const int halfWid = static_cast<int>(inTplWid * 0.5);
	const int halfHei = static_cast<int>(inTplHei * 0.5);

	std::vector<std::pair<int, std::vector<std::pair<int, int>>>> vecRowInf;
	std::vector<std::pair<int, std::vector<std::pair<int, int>>>> vecColInf;

	for (int rowNo = 0; rowNo < inSrcImg.rows; ++rowNo) {
		const uchar* rowPtr = inSrcImg.ptr<uchar>(rowNo);
		std::vector<std::pair<int, int>> segments;
		bool inSegment = false;
		int begIdx = 0;

		for (int colNo = 0; colNo < inSrcImg.cols; ++colNo) {
			if (!inSegment && rowPtr[colNo] > 0) {
				inSegment = true;
				begIdx = colNo;
			}
			if (inSegment && (rowPtr[colNo] == 0 || colNo == inSrcImg.cols - 1)) {
				int endIdx = (rowPtr[colNo] == 0) ? colNo - 1 : colNo;
				int len = endIdx - begIdx + 1;
				if (len > 2 * halfWid) {
					int newStart = begIdx + halfWid;
					int newEnd = endIdx - halfWid;
					if (newStart <= newEnd)
						segments.emplace_back(newStart, newEnd);
				}
				inSegment = false;
			}
		}

		if (!segments.empty())
			vecRowInf.emplace_back(rowNo, segments);
	}

	for (int colNo = 0; colNo < inSrcImg.cols; ++colNo) {
		std::vector<std::pair<int, int>> segments;
		bool inSegment = false;
		int begIdx = 0;

		for (int rowNo = 0; rowNo < inSrcImg.rows; ++rowNo) {
			uchar val = inSrcImg.at<uchar>(rowNo, colNo);
			if (!inSegment && val > 0) {
				inSegment = true;
				begIdx = rowNo;
			}
			if (inSegment && (val == 0 || rowNo == inSrcImg.rows - 1)) {
				int endIdx = (val == 0) ? rowNo - 1 : rowNo;
				int len = endIdx - begIdx + 1;
				if (len > 2 * halfHei) {
					int newStart = begIdx + halfHei;
					int newEnd = endIdx - halfHei;
					if (newStart <= newEnd)
						segments.emplace_back(newStart, newEnd);
				}
				inSegment = false;
			}
		}

		if (!segments.empty())
			vecColInf.emplace_back(colNo, segments);
	}

	for (const auto& rowInf : vecRowInf) {
		int r = rowInf.first;
		for (auto [colStart, colEnd] : rowInf.second) {
			for (int c = colStart; c <= colEnd; ++c) {
				bool inCol = false;
				for (const auto& colInf : vecColInf) {
					if (colInf.first == c) {
						for (auto [rowStart, rowEnd] : colInf.second) {
							if (r >= rowStart && r <= rowEnd) {
								inCol = true;
								break;
							}
						}
						break;
					}
				}
				if (inCol) {
					outMatPos.emplace_back(inLeftTop.x + c, inLeftTop.y + r);
				}
			}
		}
	}

	if (outMatPos.size() == 0) {
		for (int rowNo = 0; rowNo < inSrcImg.rows; ++rowNo) {
			for (int colNo = 0; colNo < inSrcImg.cols; ++colNo) {
				if (inSrcImg.ptr<uchar>(rowNo)[colNo]) {
					outMatPos.push_back(cv::Point(colNo, rowNo) + inLeftTop);
				}
			}
		}
	}
}

bool CTemplatePartGroup::GetConvexHullX(const cv::Mat& inMagImg, const std::vector<std::vector<cv::Mat>>& inRotTpls, const cv::Size2f& inCompSize, std::vector<cv::Point>& outDetectRange) {
	AnswerType answer = IMG_SUCCESS_ANS().SetErrCode();

	//// 计算凸包边界
	//std::vector<std::vector<cv::Point>> contours;
	//cv::findContours(inMagImg > 0.3, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
	//if (contours.size() == 0) {
	//	return false;
	//}
	//std::vector<cv::Point> contoursMerge;
	//for (int j = 0; j < contours.size(); ++j) {
	//	contoursMerge.insert(contoursMerge.end(), contours[j].begin(), contours[j].end());
	//}
	//cv::Mat tmpImg = cv::Mat::zeros(inMagImg.size(), CV_8UC1);
	//cv::drawContours(tmpImg, std::vector<std::vector<cv::Point>>{contoursMerge}, -1, cv::Scalar(255), -1);

	// 寻找非零方案
	std::vector<cv::Point> nonZeroPts;
	cv::findNonZero(inMagImg > 0.5, nonZeroPts);
	if (nonZeroPts.size() < 3) {
		answer = IMG_PARAM_ERROR_ANS().SetErrCode(IMG_PARAM_ERROR_ANS::ANS2::ConvexHullPointsNumberError);
		return false;
	}
	std::vector<cv::Point> hullPts;
	cv::convexHull(nonZeroPts, hullPts);
	cv::Rect rect = cv::boundingRect(nonZeroPts);
	int minX = inMagImg.cols - 1, minY = inMagImg.rows - 1, maxX = 0, maxY = 0;
	for (int i = 0; i < hullPts.size(); ++i) {
		if (hullPts[i].x < minX) minX = hullPts[i].x;
		if (hullPts[i].y < minY) minY = hullPts[i].y;
		if (hullPts[i].x > maxX) maxX = hullPts[i].x;
		if (hullPts[i].y > maxY) maxY = hullPts[i].y;
	}

	cv::Mat tmpImg = cv::Mat::zeros(cv::Size(maxX - minX + 1, maxY - minY + 1), CV_8UC1);
	std::vector<cv::Point> tmpHullPts;
	const cv::Point leftTop(minX, minY);
	for (int i = 0; i < hullPts.size(); ++i) {
		tmpHullPts.push_back(hullPts[i] - leftTop);
	}
	cv::drawContours(tmpImg, std::vector<std::vector<cv::Point>>{tmpHullPts}, -1, cv::Scalar(255), -1);

	// 计算缩小半径
	std::vector<double> vecTplMinX(inRotTpls.size(), 0);
	std::vector<double> vecTplMinY(inRotTpls.size(), 0);
	std::vector<double> vecTplMaxX(inRotTpls.size(), 0);
	std::vector<double> vecTplMaxY(inRotTpls.size(), 0);
	for (int tplNo = 0; tplNo < inRotTpls.size(); ++tplNo) {
		cv::minMaxLoc(inRotTpls[tplNo][0], &vecTplMinX[tplNo], &vecTplMaxX[tplNo]);
		cv::minMaxLoc(inRotTpls[tplNo][1], &vecTplMinY[tplNo], &vecTplMaxY[tplNo]);
	}

	if (vecTplMaxX.size() == 0 || vecTplMinX.size() == 0 || vecTplMaxY.size() == 0 || vecTplMinY.size() == 0) {
		return false;
	}
	const double ratio = 0.94;
	const double tplWidth = (*std::min_element(vecTplMaxX.begin(), vecTplMaxX.end()) - *std::max_element(vecTplMinX.begin(), vecTplMinX.end())) * ratio;
	const double tplHeight = (*std::min_element(vecTplMaxY.begin(), vecTplMaxY.end()) - *std::max_element(vecTplMinY.begin(), vecTplMinY.end())) * ratio;

	// 输出匹配区域
	GetContinuousRegions(tmpImg, leftTop, tplWidth, tplHeight, outDetectRange);

#ifdef _DEBUG
	cv::Mat showImg;
	cv::cvtColor(inMagImg, showImg, cv::COLOR_GRAY2BGR);
	cv::drawContours(showImg, std::vector<std::vector<cv::Point>>{hullPts}, -1, cv::Scalar(0, 0, 1), 1);  // 显示凸包
	for (int ptNo = 0; ptNo < outDetectRange.size(); ++ptNo) {
		cv::circle(showImg, outDetectRange[ptNo], 1, cv::Scalar(0, 1, 0), -1);
	}
	//cv::rectangle(showImg, cv::Point(minX, minY), cv::Point(maxX, maxY), cv::Scalar(0, 0, 1));  // 显示位置约束
#endif

	return true;
}

AnswerType CTemplatePartGroup::GetDenoiseGradientAndMagnitudeImages(std::vector<cv::Mat>& inGradImgs, cv::Mat& inMagImg, const float& inMaxMag, const float& inMagTrs) {
	AnswerType answer = IMG_SUCCESS_ANS().SetErrCode();

	const cv::Mat mask = inMagImg < std::max(4 * 6 / inMaxMag, inMagTrs);
	inMagImg.setTo(0, mask);
	inGradImgs[0].setTo(0, mask);
	inGradImgs[1].setTo(0, mask);

	return answer;
}

AnswerType CTemplatePartGroup::MatchRotatedTemplatesCoarse(const cv::Point2f& inScale, const cv::Mat& inPartImg, const std::vector<std::vector<cv::Mat>>& inRotTpls, const cv::Size2f& inTotalSize, const cv::Size2f& inMoldSize, const int& inBegPixelX, const int& inEndPixelX, const int& inBegPixelY, const int& inEndPixelY, const int& inStepPixel, const float& inBegAng, const float& inStepAng, const int& inMaxPosNum, const int& inMaxNum, cv::Mat& outCrsMagImg, std::vector<cv::Point2f>& outMatchOffsets, std::vector<float>& outMatchAngles, std::vector<double>& outMatchScores, const int& inPyrLvl) {
	AnswerType answer = IMG_SUCCESS_ANS().SetErrCode();

	//#if showTimeFlag
	//    double time_start, tmpTime;
	//#endif

	if (inRotTpls.size() == 0) {
		answer = IMG_PARAM_ERROR_ANS().SetErrCode(IMG_PARAM_ERROR_ANS::ANS2::TemplateNumberError);
		return answer;
	}

	const float totalX = inTotalSize.width * powf(0.5, inPyrLvl);
	const float totalY = inTotalSize.height * powf(0.5, inPyrLvl);
	const float moldX = inMoldSize.width * powf(0.5, inPyrLvl);
	const float moldY = inMoldSize.height * powf(0.5, inPyrLvl);

	// 计算梯度幅值图
//#if showTimeFlag
//    time_start = cv::getTickCount();  // 计时开始
//#endif
	//std::vector<cv::Mat> gradImgs;
	float maxMagVal = 0;
	//cv::Mat blurImg;
	//cv::GaussianBlur(inPartImg, blurImg, cv::Size(3, 3), 0);
	//GetNormalizedGradientAndMagnitudeImages(inPartImg, gradImgs, outCrsMagImg, true, &maxMagVal);
	//GetDenoiseGradientAndMagnitudeImages(gradImgs, outCrsMagImg, maxMagVal);
	//GetMaxBlurImage(outCrsMagImg, outCrsMagImg);
	cm::getNormalizedMagnitudeImages(inPartImg, outCrsMagImg, &maxMagVal);
	//#if showTimeFlag
	//    tmpTime = (cv::getTickCount() - time_start) * 1000. / double(cv::getTickFrequency());
	//    std::cout << " - - - 幅值梯度图: " << tmpTime << "ms" << std::endl;  // 计时结束
	//#endif


		// 计算凸包
	//#if showTimeFlag
	//    time_start = cv::getTickCount();  // 计时开始
	//#endif
	std::vector<cv::Point> detectRange;
	int beginX = inBegPixelX, beginY = inBegPixelY, endX = inEndPixelX, endY = inEndPixelY;
	const bool useHullFlag = GetConvexHullX(outCrsMagImg, inRotTpls, cv::Size2f(totalX, totalY), detectRange);
	//if (useHullFlag) {
	//	beginX = std::max(beginX, detectRange[0]);
	//	endX = std::min(endX, detectRange[2]);
	//	beginY = std::max(beginY, detectRange[1]);
	//	endY = std::min(endY, detectRange[3]);
	//}
//#if showTimeFlag
//    tmpTime = (cv::getTickCount() - time_start) * 1000. / double(cv::getTickFrequency());
//    std::cout << " - - - 计算凸包: " << tmpTime << "ms" << std::endl;  // 计时结束
//#endif

	// 计算幅值
//#if showTimeFlag
//    time_start = cv::getTickCount();  // 计时开始
//#endif
	cv::Point2f imgCenter((outCrsMagImg.cols - 1) * 0.5, (outCrsMagImg.rows - 1) * 0.5);

	const int templateNum = inRotTpls.size();
	std::vector<std::vector<matchPoint>> tmpMatchPoints(templateNum);
	std::vector<float> vecMaxScoreM(templateNum, 0);

	float diaLen = 0;
	GetShapeDiagonalLength(cv::Size2f(totalX, totalY), diaLen);
	const float nozzleOffset = 0.1 / (inScale.x + inScale.y) * 4.;
	diaLen += nozzleOffset;
	const float imgRadiusPow = powf(diaLen * 0.5, 2);

	if (!useHullFlag) {
		mFailNum += 1;
# ifdef _DEBUG
		for (int i = 0; i < templateNum; ++i) {
# else
		concurrency::parallel_for(0, templateNum, [&](int i) {
# endif
			float scoreM = 0;
			for (int x = beginX; x <= endX; x += inStepPixel) {
				for (int y = beginY; y <= endY; y += inStepPixel) {
					//if (powf(x - imgCenter.x, 2) + powf(y - imgCenter.y, 2) >= imgRadiusPow) continue;
					scoreM = cm::calculateMagnitudeScore(outCrsMagImg, inRotTpls[i][0], inRotTpls[i][1], x, y);

					if (vecMaxScoreM[i] < scoreM) vecMaxScoreM[i] = scoreM;
					if (scoreM > 0.1) {
						tmpMatchPoints[i].push_back(matchPoint(0, scoreM, 0, inBegAng + i * inStepAng, x, y));
					}
				}
			}
# ifdef _DEBUG
			}
# else
		});
# endif
	}
	else {
# ifdef _DEBUG
		for (int i = 0; i < templateNum; ++i) {
# else
		concurrency::parallel_for(0, templateNum, [&](int i) {
# endif
			float scoreM = 0;
			for (int ptNo = 0; ptNo < detectRange.size(); ++ptNo) {
				scoreM = cm::calculateMagnitudeScore(outCrsMagImg, inRotTpls[i][0], inRotTpls[i][1], detectRange[ptNo].x, detectRange[ptNo].y);

				if (vecMaxScoreM[i] < scoreM) vecMaxScoreM[i] = scoreM;
				if (scoreM > 0.1) {
					tmpMatchPoints[i].push_back(matchPoint(0, scoreM, 0, inBegAng + i * inStepAng, detectRange[ptNo].x, detectRange[ptNo].y));
				}
			}
# ifdef _DEBUG
			}
# else
		});
# endif
	}
	//#if showTimeFlag
	//    tmpTime = (cv::getTickCount() - time_start) * 1000. / double(cv::getTickFrequency());
	//    std::cout << " - - - 计算幅值: " << tmpTime << "ms" << std::endl;  // 计时结束
	//#endif


		// 幅值筛选
	//#if showTimeFlag
	//    time_start = cv::getTickCount();  // 计时开始
	//#endif
	const float maxScoreM = *std::max_element(vecMaxScoreM.begin(), vecMaxScoreM.end());
	//std::vector<std::vector<matchPoint>> matchPoints(templateNum);
	std::vector<matchPoint> gradientSelectPoints;
	for (int i = 0; i < templateNum; ++i) {
		for (int j = 0; j < tmpMatchPoints[i].size(); ++j) {
			if (tmpMatchPoints[i][j].scoreM > std::max(maxScoreM * 0.6, vecMaxScoreM[i] * 0.9)) {
				tmpMatchPoints[i][j].scoreM /= maxScoreM;
				gradientSelectPoints.push_back(tmpMatchPoints[i][j]);
			}
		}
	}
	//const int useNum = round(templateNum);
	//int maxNum = 0;
	//int begIdx = 0;
	//for (int i = 0; i < templateNum - useNum + 1; ++i) {
	//    int tmpNum = 0;
	//    for (int j = 0; j < useNum; ++j) {
	//        tmpNum += matchPoints[i + j].size();
	//    }
	//    if (tmpNum > maxNum) {
	//        maxNum = tmpNum;
	//        begIdx = i;
	//    }
	//}
	//for (int i = 0; i < begIdx; ++i) {
	//    matchPoints[i].clear();
	//}
	//for (int i = begIdx + useNum; i < templateNum; ++i) {
	//    matchPoints[i].clear();
	//}
	int pointNum = 0;
	//for (int i = 0; i < matchPoints.size(); i++) {
	//    pointNum += matchPoints[i].size();
	//}
	if (gradientSelectPoints.size() == 0) {
		answer = IMG_MATCH_FAIL_ANS().SetErrCode(IMG_MATCH_FAIL_ANS::ANS2::CandidatePointsEmpty);
		return answer;
	}
	//#if showTimeFlag
	//    tmpTime = (cv::getTickCount() - time_start) * 1000. / double(cv::getTickFrequency());
	//    std::cout << " - - - 幅值筛选: " << tmpTime << "ms" << std::endl;  // 计时结束
	//#endif

		/*
		// 计算梯度
	#if showTimeFlag
		time_start = cv::getTickCount();  // 计时开始
	#endif
		std::vector<float> vecMaxScoreG(templateNum, 0);
	# ifdef _DEBUG
		for (int i = 0; i < templateNum; i++) {
	# else
		concurrency::parallel_for(0, templateNum, [&](int i) {
	# endif
			float scoreG = 0.;
			for (int j = 0; j < matchPoints[i].size(); j++) {
				scoreG = CalculateGradientScore(gradImgs[0], gradImgs[1], inRotTpls[i][0], inRotTpls[i][1], inRotTpls[i][2], inRotTpls[i][3], matchPoints[i][j].x, matchPoints[i][j].y);
				if (scoreG > vecMaxScoreG[i]) vecMaxScoreG[i] = scoreG;
				matchPoints[i][j].scoreG = scoreG;
			}
	# ifdef _DEBUG
			}
	# else
		});
	# endif
	#if showTimeFlag
		tmpTime = (cv::getTickCount() - time_start) * 1000. / double(cv::getTickFrequency());
		std::cout << " - - - 计算梯度: " << tmpTime << "ms" << std::endl;  // 计时结束
	#endif


		// 梯度筛选
	#if showTimeFlag
		time_start = cv::getTickCount();  // 计时开始
	#endif
		const float maxScoreG = *std::max_element(vecMaxScoreG.begin(), vecMaxScoreG.end());
		std::vector<std::vector<matchPoint>> gradientPoints(templateNum);
		for (int i = 0; i < templateNum; i++) {
			for (int j = 0; j < matchPoints[i].size(); j++) {
				if (matchPoints[i][j].scoreG > std::min(0.9 * vecMaxScoreG[i], 0.6 * maxScoreG)) {
					gradientPoints[i].push_back(matchPoint(matchPoints[i][j].scoreG / maxScoreG, matchPoints[i][j].scoreM, 0, matchPoints[i][j].angle, matchPoints[i][j].x, matchPoints[i][j].y));
				}
			}
		}

		pointNum = 0;
		for (int i = 0; i < gradientPoints.size(); i++) {
			pointNum += gradientPoints[i].size();
		}
		if (pointNum == 0) {
			answer = IMG_RecogTgtFeatDetectErr_ANS().SetErrCode(IMG_RecogTgtFeatDetectErr_ANS::ANS2::BoxCandidatePointsNotMatched);
			return answer;
		}
		*/

		//std::vector<matchPoint> gradientSelectPoints;
		//// 计算总元素数量
		//size_t totalSize = 0;
		//for (const auto& v : matchPoints) totalSize += v.size();  // gradientPoints
		//gradientSelectPoints.reserve(totalSize);
		//
		//
		//// 展平
		//for (const auto& v : matchPoints)  // gradientPoints
		//    gradientSelectPoints.insert(gradientSelectPoints.end(), v.begin(), v.end());

	//# ifdef _DEBUG
	//    CheckMatchPoints(outCrsMagImg, gradientSelectPoints);
	//# endif
	//#if showTimeFlag
	//    tmpTime = (cv::getTickCount() - time_start) * 1000. / double(cv::getTickFrequency());
	//    std::cout << " - - - 梯度筛选: " << tmpTime << "ms" << std::endl;  // 计时结束
	//#endif


		// 相同位置筛选
	//#if showTimeFlag
	//    time_start = cv::getTickCount();  // 计时开始
	//#endif
	std::vector<matchPoint> posPoints;
	std::vector<matchPoint> tempPoints;
	std::sort(gradientSelectPoints.begin(), gradientSelectPoints.end(), [](matchPoint mp1, matchPoint mp2) {if (mp1.x == mp2.x) { return mp1.y > mp2.y; } else { return mp1.x > mp2.x; }});
	for (int i = 0; i < gradientSelectPoints.size(); i++) {
		if (i == 0) {
			tempPoints.push_back(gradientSelectPoints[i]);
		}
		else if (gradientSelectPoints[i].x == tempPoints[0].x && gradientSelectPoints[i].y == tempPoints[0].y) {
			tempPoints.push_back(gradientSelectPoints[i]);
		}
		else
		{
			std::sort(tempPoints.begin(), tempPoints.end(), [](matchPoint mp1, matchPoint mp2) {return mp1.scoreM > mp2.scoreM; });
			for (int j = 0; j < std::min(inMaxPosNum, int(tempPoints.size())); ++j) {
				posPoints.push_back(tempPoints[j]);
			}
			tempPoints.clear();
			tempPoints.push_back(gradientSelectPoints[i]);
		}
		if (i == (gradientSelectPoints.size() - 1)) {
			std::sort(tempPoints.begin(), tempPoints.end(), [](matchPoint mp1, matchPoint mp2) {return mp1.scoreM > mp2.scoreM; });
			for (int j = 0; j < std::min(inMaxPosNum, int(tempPoints.size())); ++j) {
				posPoints.push_back(tempPoints[j]);
			}
		}
	}
	if (posPoints.size() == 0) {
		answer = IMG_MATCH_FAIL_ANS().SetErrCode(IMG_MATCH_FAIL_ANS::ANS2::CandidatePointsEmpty);
		return answer;
	}
# ifdef _DEBUG
	CheckMatchPoints(outCrsMagImg, posPoints);
# endif
	//#if showTimeFlag
	//    tmpTime = (cv::getTickCount() - time_start) * 1000. / double(cv::getTickFrequency());
	//    std::cout << " - - - 相同位置筛选: " << tmpTime << "ms" << std::endl;  // 计时结束
	//#endif


		// 最大数量筛选
	//#if showTimeFlag
	//    time_start = cv::getTickCount();  // 计时开始
	//#endif
	const int maxSelAngNum = 2;  // round(useNum * 0.5);
	const int sglAngPosNum = 1;
	std::vector<matchPoint> selectPoints;
	std::sort(posPoints.begin(), posPoints.end(), [](matchPoint mp1, matchPoint mp2) { return mp1.scoreM > mp2.scoreM; });
	std::vector<int> selAngNo;
	std::vector<int> selAngNum;
	for (int ptNo = 0; ptNo < posPoints.size(); ++ptNo) {
		int angNo = cvRound(float(posPoints[ptNo].angle - inBegAng) / inStepAng);
		if (std::find(selAngNo.begin(), selAngNo.end(), angNo) == selAngNo.end() && selAngNo.size() < maxSelAngNum) {
			selAngNo.push_back(angNo);
			selAngNum.push_back(0);
		}
		auto angItr = std::find(selAngNo.begin(), selAngNo.end(), angNo);
		if (angItr != selAngNo.end()) {
			int angIdx = std::distance(selAngNo.begin(), angItr);
			if (selAngNum[angIdx] < sglAngPosNum) {
				++selAngNum[angIdx];
				selectPoints.push_back(posPoints[ptNo]);
			}
		}
	}

# ifdef _DEBUG
	CheckMatchPoints(outCrsMagImg, selectPoints);
# endif
	//#if showTimeFlag
	//    tmpTime = (cv::getTickCount() - time_start) * 1000. / double(cv::getTickFrequency());
	//    std::cout << " - - - 最大数量筛选: " << tmpTime << "ms" << std::endl;  // 计时结束
	//#endif

	for (int i = 0; i < selectPoints.size(); i++) {
		outMatchOffsets.push_back(cv::Point2f((selectPoints[i].x - imgCenter.x) * powf(2, inPyrLvl), (selectPoints[i].y - imgCenter.y) * powf(2, inPyrLvl)));
		outMatchAngles.push_back(selectPoints[i].angle);
		outMatchScores.push_back(selectPoints[i].scoreM);
	}

	return answer;
}

AnswerType CTemplatePartGroup::MatchRotatedTemplatesNormal(const cv::Size2f& inTotalSize, const cv::Mat& inPartImg, const std::vector<std::vector<cv::Mat>>& inRotTpls, const float& inBegAng, std::vector<cv::Point2f>& inOffsets, const int& inMargin, std::vector<float>& inAngles, const float& inAngleRange, const int& inStepPixel, const float& inStepAng, cv::Point2f& outOffset, float& outAngle, const int& inPyrLvl) {
	AnswerType answer = IMG_SUCCESS_ANS().SetErrCode();

	//#if showTimeFlag
	//    double time_start, tmpTime;
	//#endif

		// 截取匹配区域
	//#if showTimeFlag
	//    time_start = cv::getTickCount();  // 计时开始
	//#endif
	const float totalX = inTotalSize.width * powf(0.5, inPyrLvl) + 8;
	const float totalY = inTotalSize.height * powf(0.5, inPyrLvl) + 8;
	const cv::Point2f imgCenter((inPartImg.cols - 1) * 0.5, (inPartImg.rows - 1) * 0.5);
	std::vector<cv::Point2f> vecVertexes = { cv::Point(-0.5 * totalX, -0.5 * totalY), cv::Point(0.5 * totalX, -0.5 * totalY) , cv::Point(0.5 * totalX, 0.5 * totalY), cv::Point(-0.5 * totalX, 0.5 * totalY) };
	std::vector<std::vector<cv::Point2f>> vecMatchVertexes;
	int begX = inPartImg.cols, begY = inPartImg.rows, endX = 0, endY = 0;
	for (int angNo = 0; angNo < inAngles.size(); ++angNo) {
		const float radAng = inAngles[angNo] * CV_PI / float(180);
		const float sinT = std::sinf(radAng), cosT = std::cosf(radAng);
		std::vector<cv::Point2f> tmpVertexes;
		for (int ptNo = 0; ptNo < vecVertexes.size(); ++ptNo) {
			tmpVertexes.push_back(cv::Point2f(cosT * vecVertexes[ptNo].x - sinT * vecVertexes[ptNo].y + imgCenter.x + inOffsets[angNo].x * powf(0.5, inPyrLvl),
				sinT * vecVertexes[ptNo].x + cosT * vecVertexes[ptNo].y + imgCenter.y + inOffsets[angNo].y * powf(0.5, inPyrLvl)));
			if (floor(tmpVertexes[ptNo].x) < begX) begX = floor(tmpVertexes[ptNo].x);
			if (floor(tmpVertexes[ptNo].y) < begY) begY = floor(tmpVertexes[ptNo].y);
			if (ceil(tmpVertexes[ptNo].x) > endX) endX = ceil(tmpVertexes[ptNo].x);
			if (ceil(tmpVertexes[ptNo].y) > endY) endY = ceil(tmpVertexes[ptNo].y);
		}
	}
	cv::Rect cropRect = cv::Rect(begX, begY, endX - begX + 1, endY - begY + 1) & cv::Rect(0, 0, inPartImg.cols, inPartImg.rows);
	cv::Mat cropImg;// = inPartImg(cropRect).clone();
	cv::GaussianBlur(inPartImg(cropRect), cropImg, cv::Size(3, 3), 0);
	//#if showTimeFlag
	//    tmpTime = (cv::getTickCount() - time_start) * 1000. / double(cv::getTickFrequency());
	//    std::cout << " - - - 截取匹配区域: " << tmpTime << "ms" << std::endl;  // 计时结束
	//#endif

		// 计算梯度幅值图
	//#if showTimeFlag
	//    time_start = cv::getTickCount();  // 计时开始
	//# endif
	cv::Mat magImg;
	std::vector<cv::Mat> gradImgs;
	cv::Mat grayImg;
	cm::getNormalizedGradientAndMagnitudeImages(cropImg, gradImgs, magImg, true);
	cm::getMaxBlurImage(magImg, magImg);
	cm::getNormalizedGrayImage(cropImg, grayImg);
	//#if showTimeFlag
	//    tmpTime = (cv::getTickCount() - time_start) * 1000. / double(cv::getTickFrequency());
	//    std::cout << " - - - 梯度幅值图: " << tmpTime << "ms" << std::endl;  // 计时结束
	//#endif

		// 将匹配中心转换至当前坐标系下
	//#if showTimeFlag
	//    time_start = cv::getTickCount();  // 计时开始
	//#endif
	std::vector<cv::Point> vecMatchCenter(inOffsets.begin(), inOffsets.end());
	for (int i = 0; i < vecMatchCenter.size(); ++i) {
		vecMatchCenter[i] = cv::Point(cvRound(imgCenter.x + vecMatchCenter[i].x * powf(0.5, inPyrLvl) - cropRect.x), cvRound(imgCenter.y + vecMatchCenter[i].y * powf(0.5, inPyrLvl) - cropRect.y));
	}

	int minX = cropImg.cols - 1, maxX = 0, minY = cropImg.rows - 1, maxY = 0;
	for (int i = 0; i < vecMatchCenter.size(); ++i) {
		if ((vecMatchCenter[i].x - inMargin) < minX) minX = vecMatchCenter[i].x - inMargin;
		if ((vecMatchCenter[i].x + inMargin) > maxX) maxX = vecMatchCenter[i].x + inMargin;
		if ((vecMatchCenter[i].y - inMargin) < minY) minY = vecMatchCenter[i].y - inMargin;
		if ((vecMatchCenter[i].y + inMargin) > maxY) maxY = vecMatchCenter[i].y + inMargin;
	}
	const cv::Size matchRegionSize(maxX - minX + 1, maxY - minY + 1);
	const cv::Point matOffset(minX, minY);

	std::vector<int> matchFlags(inRotTpls.size(), 0);
	std::vector<cv::Mat> matchRegions(inRotTpls.size());
	for (int i = 0; i < vecMatchCenter.size(); ++i) {
		const int angleBeginIndex = round((inAngles[i] - inAngleRange - inBegAng) / inStepAng);
		const int angleEndIndex = round((inAngles[i] + inAngleRange - inBegAng) / inStepAng);
		for (int j = angleBeginIndex; j <= angleEndIndex; ++j) {
			if (matchFlags[j] == 0) {
				matchFlags[j] = 1;
				matchRegions[j] = cv::Mat::zeros(matchRegionSize, CV_8UC1);
			}
			cv::circle(matchRegions[j], vecMatchCenter[i] - matOffset, inMargin, 1, -1);
		}
	}
	//#if showTimeFlag
	//    tmpTime = (cv::getTickCount() - time_start) * 1000. / double(cv::getTickFrequency());
	//    std::cout << " - - - 匹配位置转换: " << tmpTime << "ms" << std::endl;  // 计时结束
	//#endif

		// 计算相似度
	//#if showTimeFlag
	//    time_start = cv::getTickCount();  // 计时开始
	//#endif
	std::vector<std::vector<matchPoint>> vecMatRes(inRotTpls.size());
	std::vector<float> vecMaxScoreM(inRotTpls.size(), 0);
	std::vector<float> vecMaxScoreG(inRotTpls.size(), 0);
	std::vector<float> vecMaxScoreV(inRotTpls.size(), 0);
# ifdef _DEBUG
	for (int i = 0; i < inRotTpls.size(); ++i) {
# else
	concurrency::parallel_for(0, int(inRotTpls.size()), [&](int i) {
# endif
		if (matchFlags[i] == 1) {
			const uchar* pRegion = matchRegions[i].data;
			for (int row = 0; row < matchRegionSize.height; ++row) {
				float scoreM = 0, scoreG = 0, scoreV = 0;
				int x, y;
				for (int col = 0; col < matchRegionSize.width; ++col) {
					if (*(pRegion + row * matchRegions[i].cols + col) == 1) {
						x = minX + col;
						y = minY + row;
						cm::calculateMagnitudeAndGradientScore(magImg, gradImgs[0], gradImgs[1], inRotTpls[i][0], inRotTpls[i][1], inRotTpls[i][2], inRotTpls[i][3], x, y, scoreM, scoreG);
						if (inRotTpls[i].size() == 6) {
							scoreV = cm::calculateGrayScore(grayImg, inRotTpls[i][4], inRotTpls[i][5], x, y);
						}
						vecMatRes[i].push_back(matchPoint(scoreG, scoreM, scoreV, i, x, y));
						if (scoreM > vecMaxScoreM[i]) vecMaxScoreM[i] = scoreM;
						if (scoreG > vecMaxScoreG[i]) vecMaxScoreG[i] = scoreG;
						if (scoreV > vecMaxScoreV[i]) vecMaxScoreV[i] = scoreV;
					}
				}
			}
		}
# ifdef _DEBUG
		}
# else
	});
# endif
	//#if showTimeFlag
	//    tmpTime = (cv::getTickCount() - time_start) * 1000. / double(cv::getTickFrequency());
	//    std::cout << " - - - 相似度计算: " << tmpTime << "ms" << std::endl;  // 计时结束
	//#endif

		// 结果导出
	//#if showTimeFlag
	//    time_start = cv::getTickCount();  // 计时开始
	//#endif
	const float maxM = *std::max_element(vecMaxScoreM.begin(), vecMaxScoreM.end());
	const float maxG = *std::max_element(vecMaxScoreG.begin(), vecMaxScoreG.end());
	const float maxV = *std::max_element(vecMaxScoreV.begin(), vecMaxScoreV.end());
	const float fM = maxM < 0.001 ? 0 : 1. / maxM;
	const float fG = maxG < 0.001 ? 0 : 1. / maxG;
	const float fV = maxV < 0.001 ? 0 : 1. / maxV;

	float maxScore = 0;
	float tmpScore = 0;
	int maxPtNo = -1;
	int maxPosNo = -1;
	for (int ptNo = 0; ptNo < vecMatRes.size(); ++ptNo) {
		for (int posNo = 0; posNo < vecMatRes[ptNo].size(); ++posNo) {
			if (inRotTpls[0].size() == 4) {
				tmpScore = vecMatRes[ptNo][posNo].scoreM * fM * 0.5 + vecMatRes[ptNo][posNo].scoreG * fG * 0.5;
			}
			else {
				tmpScore = vecMatRes[ptNo][posNo].scoreG * fG * 0.4 + vecMatRes[ptNo][posNo].scoreM * fM * 0.4 + vecMatRes[ptNo][posNo].scoreV * fV * 0.2;
				//tmpScore = vecMatRes[ptNo][posNo].scoreG * fG * 0.5 + vecMatRes[ptNo][posNo].scoreM * fM * 0.5;
				//tmpScore = vecMatRes[ptNo][posNo].scoreG * fG;
			}

			if (tmpScore > maxScore) {
				maxScore = tmpScore;
				maxPtNo = ptNo;
				maxPosNo = posNo;
			}
		}
	}

	if (maxScore == 0 || tmpScore == 0 || maxPtNo == -1 || maxPosNo == -1) {
		answer = IMG_MATCH_FAIL_ANS().SetErrCode(IMG_MATCH_FAIL_ANS::ANS2::CandidatePointsEmpty);;
		return answer;
	}

	outOffset = cv::Point2f((vecMatRes[maxPtNo][maxPosNo].x + cropRect.x - imgCenter.x) * powf(2, inPyrLvl), (vecMatRes[maxPtNo][maxPosNo].y + cropRect.y - imgCenter.y) * powf(2, inPyrLvl));
	outAngle = inBegAng + vecMatRes[maxPtNo][maxPosNo].angle * inStepAng;
	//#if showTimeFlag
	//    tmpTime = (cv::getTickCount() - time_start) * 1000. / double(cv::getTickFrequency());
	//    std::cout << " - - - 结果导出: " << tmpTime << "ms" << std::endl;  // 计时结束
	//#endif

	return answer;
}

AnswerType CTemplatePartGroup::MatchRotatedTemplatesPrecise(const cv::Size2f& inTotalSize, const cv::Mat& inPartImg, const std::vector<std::vector<cv::Mat>>& inMultiScaleTpls, cv::Point2f& inOffset, const int& inMargin, float& inAngle, const float& inAngleRange, const float& inStepPixel, const float& inStepAng, cv::Point2f& outOffset, float& outAngle, double& outScore, cv::Mat& outCropImg, std::vector<cv::Mat>& outCropGradImgs, cv::Mat& outCropMagImg, cv::Point& outLeftTop, float& outMaxMagVal, const int& inPyrLvl, const cv::Point2d& inSclFac) {
	AnswerType answer = IMG_SUCCESS_ANS().SetErrCode();

	//#if showTimeFlag
	//    double time_start, tmpTime;
	//#endif

		// 截取匹配区域
	//#if showTimeFlag
	//    time_start = cv::getTickCount();  // 计时开始
	//#endif
	const float totalX = inTotalSize.width * 1.1 * powf(0.5, inPyrLvl) + 10;
	const float totalY = inTotalSize.height * 1.1 * powf(0.5, inPyrLvl) + 10;
	const cv::Point2f imgCenter((inPartImg.cols - 1) * 0.5, (inPartImg.rows - 1) * 0.5);
	std::vector<cv::Point2f> vecVertexes = { cv::Point(-0.5 * totalX, -0.5 * totalY), cv::Point(0.5 * totalX, -0.5 * totalY) , cv::Point(0.5 * totalX, 0.5 * totalY), cv::Point(-0.5 * totalX, 0.5 * totalY) };
	int begX = inPartImg.cols, begY = inPartImg.rows, endX = 0, endY = 0;
	const float radAng = inAngle * CV_PI / float(180);
	const float sinT = std::sinf(radAng), cosT = std::cosf(radAng);
	std::vector<cv::Point2f> tmpVertexes;
	for (int ptNo = 0; ptNo < vecVertexes.size(); ++ptNo) {
		tmpVertexes.push_back(cv::Point2f(cosT * vecVertexes[ptNo].x - sinT * vecVertexes[ptNo].y + imgCenter.x + inOffset.x * powf(0.5, inPyrLvl),
			sinT * vecVertexes[ptNo].x + cosT * vecVertexes[ptNo].y + imgCenter.y + inOffset.y * powf(0.5, inPyrLvl)));
		if (floor(tmpVertexes[ptNo].x) < begX) begX = floor(tmpVertexes[ptNo].x);
		if (floor(tmpVertexes[ptNo].y) < begY) begY = floor(tmpVertexes[ptNo].y);
		if (ceil(tmpVertexes[ptNo].x) > endX) endX = ceil(tmpVertexes[ptNo].x);
		if (ceil(tmpVertexes[ptNo].y) > endY) endY = ceil(tmpVertexes[ptNo].y);
	}
	cv::Rect cropRect = cv::Rect(begX, begY, endX - begX + 1, endY - begY + 1) & cv::Rect(0, 0, inPartImg.cols, inPartImg.rows);
	cv::GaussianBlur(inPartImg(cropRect), outCropImg, cv::Size(3, 3), 0);
	//#if showTimeFlag
	//    tmpTime = (cv::getTickCount() - time_start) * 1000. / double(cv::getTickFrequency());
	//    std::cout << " - - - 截取匹配区域: " << tmpTime << "ms" << std::endl;  // 计时结束
	//#endif

		// 计算梯度幅值图
	//#if showTimeFlag
	//    time_start = cv::getTickCount();  // 计时开始
	//#endif
	outCropGradImgs.clear();
	cm::getNormalizedGradientAndMagnitudeImages(outCropImg, outCropGradImgs, outCropMagImg, true, &outMaxMagVal);
	outLeftTop = cv::Point(cropRect.x, cropRect.y);
	//#if showTimeFlag
	//    tmpTime = (cv::getTickCount() - time_start) * 1000. / double(cv::getTickFrequency());
	//    std::cout << " - - - 梯度幅值图: " << tmpTime << "ms" << std::endl;  // 计时结束
	//#endif

		// 将匹配中心转换至当前坐标系下
	//#if showTimeFlag
	//    time_start = cv::getTickCount();  // 计时开始
	//#endif
	const cv::Point2f matchCenter(imgCenter.x + inOffset.x * powf(0.5, inPyrLvl) - cropRect.x, imgCenter.y + inOffset.y * powf(0.5, inPyrLvl) - cropRect.y);
	const float beginAngle = inAngle - inAngleRange;
	const float endAngle = inAngle + inAngleRange;

	const float matchRadiusPow = powf((inMargin + 0.5), 2);
	matchPoint bestMatchPoint(0, 0, 0, 0, 0, 0);
	for (int scaleNo = 0; scaleNo < inMultiScaleTpls.size(); ++scaleNo) {
		std::vector<std::vector<cv::Mat>> tmpRotTpls;
		GetRotatedShapeTemplates(inMultiScaleTpls[scaleNo], tmpRotTpls, beginAngle, endAngle, inStepAng);
		GetScaleShapeTemplates(tmpRotTpls, inSclFac);
		std::vector<matchPoint> vecMatchPointsTpl(tmpRotTpls.size());
# ifdef _DEBUG
		for (int tplNo = 0; tplNo < tmpRotTpls.size(); ++tplNo) {
# else
		concurrency::parallel_for(0, int(tmpRotTpls.size()), [&](int tplNo) {
# endif
			for (float x = matchCenter.x - inMargin; x <= matchCenter.x + inMargin; x += inStepPixel) {
				for (float y = matchCenter.y - inMargin; y <= matchCenter.y + inMargin; y += inStepPixel) {
					if (powf(x - matchCenter.x, 2) + powf(y - matchCenter.y, 2) > matchRadiusPow) continue;
					float scoreG = cm::calculateGradientScore(outCropGradImgs[0], outCropGradImgs[1], tmpRotTpls[tplNo][0], tmpRotTpls[tplNo][1], tmpRotTpls[tplNo][2], tmpRotTpls[tplNo][3], x, y);
					if (scoreG > vecMatchPointsTpl[tplNo].scoreG) vecMatchPointsTpl[tplNo] = matchPoint(scoreG, 0, 0, tplNo, x, y);
					//float scoreG, scoreM;
					//cm::calculateMagnitudeAndGradientScore(outCropMagImg, outCropGradImgs[0], outCropGradImgs[1], tmpRotTpls[tplNo][0], tmpRotTpls[tplNo][1], tmpRotTpls[tplNo][2], tmpRotTpls[tplNo][3], x, y, scoreM, scoreG);
					//if ((scoreG + scoreM) > (vecMatchPointsTpl[tplNo].scoreG + vecMatchPointsTpl[tplNo].scoreM)) vecMatchPointsTpl[tplNo] = matchPoint(scoreG, scoreM, 0, tplNo, x, y);
				}
			}
# ifdef _DEBUG
			}
# else
		});
# endif
		for (int tplNo = 0; tplNo < tmpRotTpls.size(); ++tplNo) {
			if (vecMatchPointsTpl[tplNo].scoreG > bestMatchPoint.scoreG) bestMatchPoint = vecMatchPointsTpl[tplNo];
			//if ((vecMatchPointsTpl[tplNo].scoreG + vecMatchPointsTpl[tplNo].scoreM) > (bestMatchPoint.scoreG + bestMatchPoint.scoreM)) bestMatchPoint = vecMatchPointsTpl[tplNo];
		}
	}
	//#if showTimeFlag
	//    tmpTime = (cv::getTickCount() - time_start) * 1000. / double(cv::getTickFrequency());
	//    std::cout << " - - - 相似度计算: " << tmpTime << "ms" << std::endl;  // 计时结束
	//#endif

	outOffset = cv::Point2f((bestMatchPoint.x - imgCenter.x + cropRect.x) * std::pow(2, inPyrLvl), (bestMatchPoint.y - imgCenter.y + cropRect.y) * std::pow(2, inPyrLvl));
	outAngle = beginAngle + bestMatchPoint.angle * inStepAng;
	outScore = bestMatchPoint.scoreG;

	return answer;
}

AnswerType CTemplatePartGroup::PartDetect(const cv::Point2f& inScale, const cv::Mat& inPartImg, const std::vector<std::vector<cv::Mat>>& inStepTemplates, const std::vector<std::vector<cv::Mat>>& inRotTemplatesCoarse, const std::vector<std::vector<cv::Mat>>& inRotTemplates, const std::vector<std::vector<cv::Mat>>& inMultiScaleTemplates, const std::vector<int>& inPyramidLevels, const std::vector<float>& inStepPixels, const float& inBeginAngle, const float& inEndAngle, const float& inAngleRange, const std::vector<float>& inStepAngles, std::vector<int> inSobelSizes, const cv::Size2f& inTotalSize, const cv::Size2f& inMoldSize, const int& inStepNum, cv::Point2f& outOffset, float& outAngle, float& outScore, cv::Mat& outCrsMagImg, cv::Mat& outCropImg, std::vector<cv::Mat>& outCropGradImgs, cv::Mat& outCropMagImg, float& outMaxMagVal, cv::Point& outLeftTop, const cv::Point2d& inSclFac) {
	AnswerType answer = IMG_SUCCESS_ANS().SetErrCode();

#if showTimeFlag
	double time_start, tmpTime;
#endif

	// 计算下采样图
#if showTimeFlag
	time_start = cv::getTickCount();  // 计时开始
#endif
	const int pyramidNum = *std::max_element(inPyramidLevels.begin(), inPyramidLevels.end());
	std::vector<cv::Mat> pyrImgs(pyramidNum + 1);
	pyrImgs[0] = inPartImg;
	cv::Size pyrImgSize = pyrImgs[0].size();
	for (int pyrLvl = 1; pyrLvl <= pyramidNum; ++pyrLvl) {
		pyrImgSize.width /= 2;
		pyrImgSize.height /= 2;
		cv::resize(pyrImgs[pyrLvl - 1], pyrImgs[pyrLvl], pyrImgSize, cv::INTER_AREA);
	}
#if showTimeFlag
    tmpTime = (cv::getTickCount() - time_start) * 1000. / double(cv::getTickFrequency());
    std::cout << " - - 下采样: " << tmpTime << "ms" << std::endl;  // 计时结束
#endif

    // 粗匹配
#if showTimeFlag
	time_start = cv::getTickCount();  // 计时开始
#endif
	int step = 0;
	const int pyramidLevel0 = inPyramidLevels[step];
	const int stepPixel0 = inStepPixels[step];
	const float stepAngle0 = inStepAngles[step];
	const float beginAngle0 = inBeginAngle + inAngleRange;
	const float endAngle0 = inEndAngle - inAngleRange;
	const int maxPosNum = 2;
	const int maxNum = 6; // 特征点数
	float rectDiaLen0 = 0.;
	GetShapeDiagonalLength(cv::Size2f(inTotalSize.width * pow(0.5, pyramidLevel0), inTotalSize.height * pow(0.5, pyramidLevel0)), rectDiaLen0);
	int margin0 = floorf(std::min(inTotalSize.width * pow(0.5, pyramidLevel0), inTotalSize.height * pow(0.5, pyramidLevel0)) * 0.5);
	std::vector<cv::Point2f> offsets0;
	std::vector<float> angles0;
	std::vector<double> scores0;
	answer = MatchRotatedTemplatesCoarse(inScale, pyrImgs[pyramidLevel0], inRotTemplatesCoarse, inTotalSize, inMoldSize, margin0, pyrImgs[pyramidLevel0].cols - margin0, margin0, pyrImgs[pyramidLevel0].rows - margin0, stepPixel0, beginAngle0, stepAngle0, maxPosNum, maxNum, outCrsMagImg, offsets0, angles0, scores0, pyramidLevel0);
	if (answer.first != ALGErrCode::IMG_SUCCESS) return answer;
#ifdef _DEBUG
	for (int i = 0; i < offsets0.size(); ++i) {
		CheckMatchTemplate(pyrImgs[inPyramidLevels[step]], inStepTemplates[step], offsets0[i], angles0[i], pyramidLevel0, 0, inSclFac);
		CheckMatchTemplate(pyrImgs[inPyramidLevels[step + 1]], inStepTemplates[step + 1], offsets0[i] * pow(2, inPyramidLevels[step] - inPyramidLevels[step + 1]), angles0[i], pyramidLevel0, 0, inSclFac);
	}
#endif
#if showTimeFlag
	tmpTime = (cv::getTickCount() - time_start) * 1000. / double(cv::getTickFrequency());
	std::cout << " - - 粗匹配: " << tmpTime << "ms" << std::endl;  // 计时结束
#endif

	// 普通匹配
#if showTimeFlag
	time_start = cv::getTickCount();  // 计时开始
#endif
	++step;
	const int pyramidLevel1 = inPyramidLevels[step];
	const int stepPixel1 = inStepPixels[step];
	const float stepAngle1 = inStepAngles[step];
	int margin1 = (stepPixel0 + 0) * pow(2, pyramidLevel0 - pyramidLevel1);
	cv::Point2f offset1;
	float angle1;
	answer = MatchRotatedTemplatesNormal(inTotalSize, pyrImgs[pyramidLevel1], inRotTemplates, inBeginAngle, offsets0, margin1, angles0, inAngleRange, stepPixel1, stepAngle1, offset1, angle1, pyramidLevel1);
	if (answer.first != ALGErrCode::IMG_SUCCESS) return answer;
#ifdef _DEBUG
	CheckMatchTemplate(pyrImgs[inPyramidLevels[step]], inStepTemplates[step], offset1, angle1, pyramidLevel1, 0, inSclFac);
	CheckMatchTemplate(pyrImgs[inPyramidLevels[step + 1]], inStepTemplates[step + 1], offset1 * pow(2, inPyramidLevels[step] - inPyramidLevels[step + 1]), angle1, pyramidLevel1, 0, inSclFac);
#endif
#if showTimeFlag
	tmpTime = (cv::getTickCount() - time_start) * 1000. / double(cv::getTickFrequency());
	std::cout << " - - 普通匹配: " << tmpTime << "ms" << std::endl;  // 计时结束
#endif

	// 精匹配
	#if showTimeFlag
	    time_start = cv::getTickCount();  // 计时开始
	#endif
	++step;
	const int pyramidLevel2 = inPyramidLevels[step];
	const float stepPixel2 = inStepPixels[step];
	const float stepAngle2 = inStepAngles[step];
	int margin2 = (stepPixel1 + 0) * pow(2, pyramidLevel1 - pyramidLevel2);
	cv::Point2f offset2;
	float angle2;
	double score2;
	answer = MatchRotatedTemplatesPrecise(inTotalSize, pyrImgs[pyramidLevel2], inMultiScaleTemplates, offset1, margin2, angle1, stepAngle1, stepPixel2, stepAngle2, offset2, angle2, score2, outCropImg, outCropGradImgs, outCropMagImg, outLeftTop, outMaxMagVal, pyramidLevel2, inSclFac);
	if (answer.first != ALGErrCode::IMG_SUCCESS) return answer;
#ifdef _DEBUG
	CheckMatchTemplate(pyrImgs[inPyramidLevels[step]], inStepTemplates[step], offset2, angle2, pyramidLevel2, 0, inSclFac);
#endif
	#if showTimeFlag
	    tmpTime = (cv::getTickCount() - time_start) * 1000. / double(cv::getTickFrequency());
	    std::cout << " - - 精匹配: " << tmpTime << "ms" << std::endl;  // 计时结束
	#endif

	outOffset = offset2;
	outAngle = angle2;
	outScore = score2 * 100;

	return answer;
}

AnswerType CTemplatePartGroup::PartResult(const cv::Point2d& inScale, const cv::Point2f& inOffset, const float& inAngle, const float& inScore, const float& inBegAng, const float& inEndAng) {
	AnswerType answer = IMG_SUCCESS_ANS().SetErrCode();

	return answer;
}

bool CTemplatePartGroup::GetCroppedImage(const cv::Mat& inSrcImg, cv::Mat& outCropImg, const cv::Point2f& inOffset, const float& inAngle, const cv::Size2f& inSize) {
	cv::Point2f imgCenter((inSrcImg.cols - 1) * 0.5, (inSrcImg.rows - 1) * 0.5);
	cv::Size cmpImgSize(ceil(inSize.width), ceil(inSize.height));
	cv::Point2f cmpCenter((cmpImgSize.width - 1) * 0.5, (cmpImgSize.height - 1) * 0.5);

	cv::Mat affMat = cv::getRotationMatrix2D(imgCenter + inOffset, inAngle, 1);
	affMat.ptr<double>(0)[2] += cmpCenter.x - (imgCenter.x + inOffset.x);
	affMat.ptr<double>(1)[2] += cmpCenter.y - (imgCenter.y + inOffset.y);

	cv::warpAffine(inSrcImg, outCropImg, affMat, cmpImgSize);

	return true;
}

AnswerType CTemplatePartGroup::GetMagnitudeErrorFlag(const cv::Mat& inMagBinImg, const std::vector<std::vector<cv::Mat>>& inConfTpls, const cv::Point2f& inOffset, const float& inAngle, const float& inMagErrT, std::vector<int>& outVecMagErrFlag) {
	AnswerType answer = IMG_SUCCESS_ANS().SetErrCode();

	cv::Point2f imgCenter((inMagBinImg.cols - 1) * 0.5, (inMagBinImg.rows - 1) * 0.5);
	std::vector<float> magScores(inConfTpls.size(), 0);
	outVecMagErrFlag = std::vector<int>(inConfTpls.size(), 0);
# ifdef _DEBUG
	for (int i = 0; i < inConfTpls.size(); ++i) {
# else
	concurrency::parallel_for(0, int(inConfTpls.size()), [&](int i) {
# endif
		std::vector<cv::Mat> tmpRotTpl;
		GetRotatedShapeTemplate(inConfTpls[i], tmpRotTpl, inAngle);
		magScores[i] = cm::calculateMagnitudeScore(inMagBinImg, tmpRotTpl[0], tmpRotTpl[1], inOffset.x, inOffset.y) / 255.;
		if (magScores[i] < inMagErrT) {
# ifdef _DEBUG
			cv::Mat showImg = inMagBinImg.clone();
			CheckMatchTemplate(showImg, tmpRotTpl, inOffset);
# endif
			outVecMagErrFlag[i] = 1;
		}
# ifdef _DEBUG
		}
# else
	});
# endif

	return answer;
}

void GetPeakCols(const cv::Mat& inMagRes, const std::vector<cv::Mat>& inTpl, const std::vector<cv::Mat>& inRightTpl, cv::Mat& outMagRes, std::vector<cv::Mat>& outTpl, std::vector<cv::Mat>& outRightTpl) {
	const int rowNum = inMagRes.rows;
	const int reqWid = 3;
	std::vector<int> vecFlag(inMagRes.cols, 0);
	for (int colNo = 0; colNo < inMagRes.cols; ++colNo) {
		int tmpMaxIdx = -1;
		float tmpMaxVal = -1;
		for (int rowNo = 0; rowNo < rowNum; ++rowNo) {
			const float tmpVal = inMagRes.ptr<float>(rowNo)[colNo];
			if (tmpVal > tmpMaxVal) {
				tmpMaxIdx = rowNo;
				tmpMaxVal = tmpVal;
			}
		}
		if (tmpMaxIdx > (reqWid - 1) && tmpMaxIdx < (rowNum - reqWid)) {
			bool flag = true;
			for (int i = 0; i < reqWid; ++i) {
				if (inMagRes.ptr<float>(tmpMaxIdx - (i))[colNo] < inMagRes.ptr<float>(tmpMaxIdx - (i + 1))[colNo] ||
					inMagRes.ptr<float>(tmpMaxIdx + (i))[colNo] < inMagRes.ptr<float>(tmpMaxIdx + (i + 1))[colNo]) {
					flag = false;
					break;
				}
			}
			if (flag) vecFlag[colNo] = 1;
		}
	}
	const int selNum = std::accumulate(vecFlag.begin(), vecFlag.end(), 0);
	outMagRes = cv::Mat(cv::Size(selNum, rowNum), inMagRes.type());
	cv::Mat tX = cv::Mat(cv::Size(selNum, 1), inMagRes.type());
	cv::Mat tY = cv::Mat(cv::Size(selNum, 1), inMagRes.type());
	cv::Mat tGX = cv::Mat(cv::Size(selNum, 1), inMagRes.type());
	cv::Mat tGY = cv::Mat(cv::Size(selNum, 1), inMagRes.type());
	cv::Mat tXR = cv::Mat(cv::Size(selNum, 1), inMagRes.type());
	cv::Mat tYR = cv::Mat(cv::Size(selNum, 1), inMagRes.type());
	int curIdx = -1;
	for (int i = 0; i < vecFlag.size(); ++i) {
		if (vecFlag[i] == 1) {
			++curIdx;
			for (int rowNo = 0; rowNo < rowNum; ++rowNo) {
				outMagRes.ptr<float>(rowNo)[curIdx] = inMagRes.ptr<float>(rowNo)[i];
			}
			tX.ptr<float>(0)[curIdx] = inTpl[0].ptr<float>(0)[i];
			tY.ptr<float>(0)[curIdx] = inTpl[1].ptr<float>(0)[i];
			tGX.ptr<float>(0)[curIdx] = inTpl[2].ptr<float>(0)[i];
			tGY.ptr<float>(0)[curIdx] = inTpl[3].ptr<float>(0)[i];
			tXR.ptr<float>(0)[curIdx] = inRightTpl[0].ptr<float>(0)[i];
			tYR.ptr<float>(0)[curIdx] = inRightTpl[1].ptr<float>(0)[i];
		}
	}
	outTpl = { tX, tY, tGX, tGY };
	outRightTpl = { tXR, tYR };
}

void DelOutliers(const cv::Mat& inDeltaMat, const std::vector<cv::Mat>& inDistMes, std::vector<cv::Mat>& outDistMes) {
	std::vector<float> vecVal(inDeltaMat.cols, 0);
	for (int colNo = 0; colNo < inDeltaMat.cols; ++colNo) {
		vecVal[colNo] = abs(inDeltaMat.ptr<float>(0)[colNo]);
	}
	std::sort(vecVal.begin(), vecVal.end());
	const float valTreshRatio = 0.8;
	const float valTresh = vecVal[int(vecVal.size() * valTreshRatio)];
	int valNum = 0;
	for (int i = 0; i < vecVal.size(); ++i) {
		if (vecVal[i] <= valTresh) {
			++valNum;
		}
	}

	outDistMes.clear();
	for (int i = 0; i < inDistMes.size(); ++i) {
		outDistMes.push_back(cv::Mat(cv::Size(valNum, 1), inDistMes[0].type()));
	}
	int curIdx = -1;
	for (int colNo = 0; colNo < inDeltaMat.cols; ++colNo) {
		if (abs(inDeltaMat.ptr<float>(0)[colNo]) <= valTresh) {
			++curIdx;
			for (int i = 0; i < inDistMes.size(); ++i) {
				outDistMes[i].ptr<float>(0)[curIdx] = inDistMes[i].ptr<float>(0)[colNo];
			}
		}
	}
}

AnswerType CTemplatePartGroup::GetPrecisePosition(const cv::Point2f& inSrcImgCtr, const cv::Point2d& inScaleFactor, const std::vector<cv::Mat>& inSrcTpl, const cv::Point2f& inScale, const cv::Size& inSrcImgSize, const cv::Mat& inCropImg, const cv::Mat& inCropMagImg, const std::vector<cv::Mat>& inCropGradImgs, const cv::Point& inLeftTop, const cv::Size2d& inTotalSize, cv::Point2f& inOffset, float& inAngle) {
	AnswerType answer = IMG_SUCCESS_ANS().SetErrCode();

#if showTimeFlag
	double time_start, tmpTime;
#endif

#if showTimeFlag
	time_start = cv::getTickCount();  // 计时开始
#endif

	// 获取粗略结果模板
	const cv::Point2f cmpCtr = inSrcImgCtr + inOffset - cv::Point2f(inLeftTop);
	std::vector<cv::Mat> tmpRotTpl;
	GetRotatedShapeTemplate(inSrcTpl, tmpRotTpl, inAngle);
	GetScaleShapeTemplate(tmpRotTpl, inScaleFactor);
	std::vector<cv::Mat> corseTpl;
	GetTranslatedShapeTemplate(tmpRotTpl, corseTpl, cmpCtr);

	// 获取对应点
	const int ipW = 4;
	cv::Mat magRes;
	cm::getInterpolationMagnitudeMap(inCropMagImg, corseTpl[0], corseTpl[1], corseTpl[2], corseTpl[3], ipW, magRes);

	// 筛选拟合区域
	cv::Mat selMagRes;
	std::vector<cv::Mat> selTpl;
	std::vector<cv::Mat> selRectTpl;
	GetPeakCols(magRes, corseTpl, inSrcTpl, selMagRes, selTpl, selRectTpl);
	cv::Mat deltaMat(cv::Size(selMagRes.cols, 1), CV_32FC1);
	if (double(selMagRes.cols) < double(magRes.cols) * 0.68) {
		return answer;
	}

# ifdef _DEBUG
	for (int colNo = 0; colNo < selMagRes.cols; ++colNo) {
# else
	concurrency::parallel_for(0, selMagRes.cols, [&](int colNo) {
# endif
		std::vector<cv::Point2f> vecPt;
		for (int i = 0; i < 2 * ipW + 1; ++i) {
			vecPt.push_back(cv::Point2f(i - ipW, selMagRes.ptr<float>(i)[colNo]));
		}

		// 三次样条插值方案 开始
		CubicSpline csCurve;
		csCurve.build(vecPt);
		const cv::Point2d tmpMaxPt = csCurve.findSplineMaxXY();
		const float delta = tmpMaxPt.x;
		// 三次样条插值方案 结束

		deltaMat.ptr<float>(0)[colNo] = delta;
# ifdef _DEBUG
		}
# else
	});
# endif
	cv::Mat resX = selTpl[0] + selTpl[2].mul(deltaMat);
	resX = cv::max(resX, 0);
	resX = cv::min(resX, inCropMagImg.cols - 1);
	cv::Mat resY = selTpl[1] + selTpl[3].mul(deltaMat);
	resY = cv::max(resY, 0);
	resY = cv::min(resY, inCropMagImg.rows - 1);
	cv::Mat resGX(resX.size(), CV_32FC1), resGY(resX.size(), CV_32FC1);
	for (int colNo = 0; colNo < resX.cols; ++colNo) {
		const int tmpX = cvRound(resX.ptr<float>()[colNo]);
		const int tmpY = cvRound(resY.ptr<float>()[colNo]);
		resGX.ptr<float>()[colNo] = inCropGradImgs[0].ptr<float>(tmpY)[tmpX];
		resGY.ptr<float>()[colNo] = inCropGradImgs[1].ptr<float>(tmpY)[tmpX];
	}

	// 删除异常点
	std::vector<cv::Mat> vecDistMes;
	//DelOutliers(deltaMat, { selRectTpl[0], selRectTpl[1], resX, resY, resGX, resGY }, vecDistMes);
	//if (double(vecDistMes[0].cols) < double(inSrcTpl[0].cols) * 0.50) {
	//	return answer;
	//}
	vecDistMes = { selRectTpl[0], selRectTpl[1], resX, resY, resGX, resGY };

	/* 最小二乘 */
	// 初始参数猜测
	Eigen::VectorXd initial_params(3);
	initial_params << cmpCtr.x, cmpCtr.y, inAngle;

	// 旧实现
	/*
	// 生成测试数据
	std::vector<Eigen::VectorXd> inputs;
	std::vector<double> outputs(resX.cols, 0);
	for (int colNo = 0; colNo < resX.cols; ++colNo) {
		Eigen::VectorXd input(6);
		input <<
			inSrcTpl[0].ptr<float>()[colNo],
			inSrcTpl[1].ptr<float>()[colNo],
			resX.ptr<float>()[colNo],
			resY.ptr<float>()[colNo],
			resGX.ptr<float>()[colNo],
			resGY.ptr<float>()[colNo];
		inputs.push_back(input);
	}

	// 创建优化器
	LMOptimizer optimizer(distanceFunction);

	// 设置优化参数
	optimizer.setLambda(0.1);
	optimizer.setLambdaFactor(2.0);
	optimizer.setMaxIterations(30);
	optimizer.setTolerance(1e-6);

	// 执行优化
	VectorXd optimized_params = optimizer.optimize(inputs, outputs, initial_params);
	*/

	// 新实现
	Eigen::VectorXd optimized_params = distanceFit(vecDistMes, initial_params, inScaleFactor);

	cv::Point2f outPreOffset(optimized_params[0] - cmpCtr.x, optimized_params[1] - cmpCtr.y);
	float outPreAngle = optimized_params[2] - inAngle;
	// 结果输出
	if (answer.first == ALGErrCode::IMG_SUCCESS &&
		abs(outPreAngle) < 3 &&
		abs(outPreOffset.x) < 5 &&
		abs(outPreOffset.y) < 5) {
		inOffset += outPreOffset;
		inAngle += outPreAngle;
	}

	return answer;
}
