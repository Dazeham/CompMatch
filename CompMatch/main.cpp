#include <ppl.h>
#include <numeric>
#include <fstream>
#include <filesystem>
#include "cm_file.hpp"
#include "cm_error_code.hpp"
#include "cm_intrinsics.hpp"
#include "cm_match_box.hpp"
#include "cm_match_lead.hpp"
#include "cm_match_ball.hpp"


int main() {
	// Init
	double time_start;
	double scale_x = 0.0222;
	double scale_y = 0.0222;

	//***   Read all component paths   ***//
	const std::string dateRoot = "..\\..\\Data";
	std::vector<std::string> vecTask = { dateRoot + "\\tra", dateRoot + "\\rot", dateRoot + "\\lig_rect", dateRoot + "\\lig_qbc" };
	std::vector<std::string> vecFolderPath;
	for (std::string taskPath : vecTask) {
		std::vector<std::string> folderPaths = cm::glob(taskPath + "\\*");
		for (std::string folderPath : folderPaths) {
			vecFolderPath.push_back(folderPath);
		}
	}

	bool stopFlag = false;
	int curNum = 0;
	int testNum = 10;

	//***   Template matching   ***//
	const std::string resRoot = "..\\res";
	std::vector<int> vecImgNum;
	for (std::string folderPath : vecFolderPath) {
		//***   Template generator   ***//
		std::shared_ptr<cm::Component> pComp = cm::GetComponentPtr(folderPath + "\\CompData.json");
		cm::CTemplatePartGroupPtr pTemplPartGroup = nullptr;
		switch (pComp->GetComponentType()) {
		case cm::ComponentType::BOX_COMPONENT: {
			pTemplPartGroup.reset(new cm::CTemplateShapeBoxType(scale_x, scale_y));
			break;
		}
		case cm::ComponentType::LEAD_COMPONENT: {
			pTemplPartGroup.reset(new cm::CTemplateShapeLead(scale_x, scale_y));
			break;
		}
		case cm::ComponentType::BALL_GROUP_COMPONENT: {
			pTemplPartGroup.reset(new cm::CTemplateShapeBall(scale_x, scale_y));
			break;
		}
		}
		pTemplPartGroup->GenerateTemplate(pComp);


		std::vector<std::string> folderItems = cm::split(folderPath, "\\");
		const std::string resPath = resRoot + "\\" + folderItems[folderItems.size()-2];
		if (!std::filesystem::exists(resPath)) {
			std::filesystem::create_directories(resPath);
		}
		const std::string resFilePath = resPath +"\\" + folderItems[folderItems.size() - 1] + ".txt";
		std::ofstream file(resFilePath);

		std::vector<float> times;
		std::vector<float> scores;
		std::vector<cv::Point2f> offsets;
		std::vector<float> angles;

		std::vector<std::string> imgPaths = cm::glob(folderPath + "\\*.bmp");
		for (int i = 0; i < imgPaths.size(); ++i) {
			AnswerType answer = IMG_SUCCESS_ANS().SetErrCode();
			//***   Read image   ***//
			cv::Mat srcImg = cv::imread(imgPaths[i], cv::IMREAD_GRAYSCALE);

			//***   Template matching   ***//
			double tmpTime = 0;
			cv::Point2f offset;
			float angle = 0.;
			float score = 0.;
			time_start = cv::getTickCount();
			answer = pTemplPartGroup->TemplateMatch(srcImg, offset, angle, score);
			tmpTime = (cv::getTickCount() - time_start) * 1000. / double(cv::getTickFrequency());
			std::cout << "匹配总时长：" << tmpTime << "ms" << std::endl;

			// Record score
			times.push_back(tmpTime);
			scores.push_back(answer.first == ALGErrCode::IMG_SUCCESS ? score : 0);
			offsets.push_back(offset);
			angles.push_back(angle);

			std::vector<std::string> items1 = cm::split(imgPaths[i], "\\");
			std::string fileName = items1[items1.size() - 1];
			file << fileName << " " << std::to_string(answer.first == ALGErrCode::IMG_SUCCESS ? score : 0) << " " << std::to_string(tmpTime) << " " <<
				std::to_string(offset.x) << " " << std::to_string(offset.y) << " " << std::to_string(angle) << "\n";

			//***   Save SVG image   ***//
			std::vector<std::string> items2 = cm::split(fileName, ".");
			std::string imgSavePath = "..\\img\\" + items1[items1.size() - 3] + "\\" +items1[items1.size() - 2];
			if (!std::filesystem::exists(imgSavePath)) {
				std::filesystem::create_directories(imgSavePath);
			}
			pTemplPartGroup->SaveResult(srcImg, offset, angle, score, answer, tmpTime, imgSavePath + "\\" + items2[0] + ".svg");

			curNum++;
			if (curNum >= testNum && stopFlag) {
				break;
			}
		}

		if (curNum >= testNum && stopFlag) {
			break;
		}
		file.close();
		vecImgNum.push_back(angles.size());
	}

	for (int i = 0; i < vecImgNum.size(); ++i) {
		std::cout << vecImgNum[i] << " ";
	}
	
	return 0;
}
