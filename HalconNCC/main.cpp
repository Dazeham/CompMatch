#include <fstream>
#include <filesystem>
#include "cm_svg.hpp"
#include "cm_file.hpp"
#include "cm_error_code.hpp"
#include "cm_comp_data.hpp"
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

	//***   Template matching   ***//
	const std::string resRoot = "..\\res";
	std::vector<int> vecImgNum;
	for (std::string folderPath : vecFolderPath) {
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
		const std::string resPath = resRoot + "\\" + folderItems[folderItems.size() - 2];
		if (!std::filesystem::exists(resPath)) {
			std::filesystem::create_directories(resPath);
		}
		const std::string resFilePath = resPath + "\\" + folderItems[folderItems.size() - 1] + ".txt";
		std::ofstream file(resFilePath);

		std::vector<float> times;
		std::vector<float> scores;
		std::vector<cv::Point2f> offsets;
		std::vector<float> angles;

		std::vector<std::string> imgPaths = cm::glob(folderPath + "\\*.bmp");
		for (int i = 0; i < imgPaths.size(); ++i) {
			AnswerType answer = IMG_SUCCESS_ANS().SetErrCode();

			//***   display image   ***//
			cv::Mat srcImg = cv::imread(imgPaths[i], cv::IMREAD_GRAYSCALE);

			//***   ReadImage   ***//
			HalconCpp::HObject SearchImage;
			ReadImage(&SearchImage, cm::vectorToHTuple({ imgPaths[i] }));
			HalconCpp::HTuple type;
			HalconCpp::GetImageType(SearchImage, &type);
	        HalconCpp::Rgb1ToGray(SearchImage, &SearchImage);

			//***   template matching   ***//
			double tmpTime = 0;
			cv::Point2f offset;
			float angle = 0.;
			float score = 0.;
			time_start = cv::getTickCount();
			answer = pTemplPartGroup->TemplateMatch(SearchImage, offset, angle, score);
			tmpTime = (cv::getTickCount() - time_start) * 1000. / double(cv::getTickFrequency());
			std::cout << "match time: " << tmpTime << "ms" << std::endl;

			// record the points
			times.push_back(tmpTime);
			scores.push_back(answer.first == ALGErrCode::IMG_SUCCESS ? score : 0);
			offsets.push_back(offset);
			angles.push_back(angle);

			std::vector<std::string> items1 = cm::split(imgPaths[i], "\\");
			std::string fileName = items1[items1.size() - 1];
			file << fileName << " " << std::to_string(answer.first == ALGErrCode::IMG_SUCCESS ? score : 0) << " " << std::to_string(tmpTime) << " " <<
				std::to_string(offset.x) << " " << std::to_string(offset.y) << " " << std::to_string(angle) << "\n";

			//***   Save SVG image   ***//
			////std::vector<std::string> items1 = split(imgPaths[i], "\\");
			////std::string fileName = items1[items1.size() - 1];
			//std::vector<std::string> items2 = cm::split(fileName, ".");
			//std::string imgSavePath = "..\\res\\";// +items1[items1.size() - 2];
			//if (!std::filesystem::exists(imgSavePath)) {
			//	std::filesystem::create_directories(imgSavePath);
			//}
			//boxMatch.SaveResult(srcImg, offset, angle, score, answer, tmpTime, imgSavePath + "\\" + items2[0] + ".svg");
		}

		file.close();
		vecImgNum.push_back(angles.size());
	}

	for (int i = 0; i < vecImgNum.size(); ++i) {
		std::cout << vecImgNum[i] << " ";
	}

	return 0;
}
