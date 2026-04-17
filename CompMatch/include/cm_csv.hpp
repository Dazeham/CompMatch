#pragma once
#ifndef CM_CSV_HPP
#define CM_CSV_HPP
#include <vector>
#include <string>
#include <fstream>
#include <opencv2/opencv.hpp>


class CSVTool {
public:
	CSVTool(const std::string& inCSVPath) {
		mCSVPath = inCSVPath;
	}
	void Export(const std::vector<double>& inVecX, const std::vector<double>& inVecY)
	{
		std::ofstream file(mCSVPath);
		file << "x,y\n";
		for (size_t i = 0; i < inVecX.size(); ++i)
			file << inVecX[i] << "," << inVecY[i] << "\n";
		file.close();
	}
	void Export(const std::vector<cv::Point2f>& inVecPt)
	{
		std::ofstream file(mCSVPath);
		file << "x,y\n";
		for (size_t i = 0; i < inVecPt.size(); ++i)
			file << inVecPt[i].x << "," << inVecPt[i].y << "\n";
		file.close();
	}
private:
	std::string mCSVPath;
};

#endif
