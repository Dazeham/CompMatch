#include <fstream>
#include <filesystem>
#include <opencv2/opencv.hpp>
#include "cm_file.hpp"


int main() {
	const cv::Size imgSize(512, 512);
	const std::string dataRoot = "..\\res";
	std::vector<std::string> vecMethod = cm::glob(dataRoot + "\\*");
	const std::vector<double> vecRotAng = { -10, -5, 5, 10, 0 };
	const std::vector<std::string> vecRotFile = { "-10", "-5", "5", "10", "0" };
	const std::vector<cv::Point2d> vecTraOfs = { cv::Point2d(0, -10), cv::Point2d(0, 10), cv::Point2d(-10, 0), cv::Point2d(10, 0), cv::Point2d(0, 0) };
	const std::vector<std::string> vecTraFile = { "up", "down", "left", "right", "mid" };
	const std::string csvRoot = "..\\csv";
	if (!std::filesystem::exists(csvRoot)) {
		std::filesystem::create_directories(csvRoot);
	}
	const double fac = 1.960;  // 95%

	/*  Rotation  */
	// Extracted results
	std::vector<std::vector<std::vector<double>>> vecRotMes(vecMethod.size());
	std::vector<std::vector<std::vector<double>>> vecRotTime(vecMethod.size());
	std::vector<std::vector<std::vector<int>>> vecPassFlag(vecMethod.size());
	for (int mNo = 0; mNo < vecMethod.size(); ++mNo) {
		std::vector<std::vector<double>> tmpAngs(vecRotAng.size());
		std::vector<std::vector<double>> tmpTimes(vecRotAng.size());
		std::vector<std::vector<int>> tmpPassFlags(vecRotAng.size());
		for (int aNo = 0; aNo < vecRotAng.size(); ++aNo) {
			std::vector<double> tmpImgs;
			std::vector<double> tmpTime;
			std::vector<int> tmpFlags;
			std::string filePath = dataRoot + "\\" + vecMethod[mNo] + "\\" + "rot" + "\\" + vecRotFile[aNo] + ".txt";
			std::ifstream file(filePath);
			std::string line;
			while (std::getline(file, line)) {
				std::vector<std::string> items = cm::split(line, " ");
				tmpImgs.push_back(std::stod(items[items.size()-1]));
				tmpTime.push_back(std::stod(items[2]));
				tmpFlags.push_back(std::stod(items[1]) > 50 ? 1 : 0);
				if (std::stod(items[1]) < 50) {
					int j = 0;
				}
			}
			tmpAngs[aNo] = tmpImgs;
			tmpTimes[aNo] = tmpTime;
			tmpPassFlags[aNo] = tmpFlags;
		}
		vecRotMes[mNo] = tmpAngs;
		vecRotTime[mNo] = tmpTimes;
		vecPassFlag[mNo] = tmpPassFlags;
	}
	
	// Calculate the angle of relative rotation
	const int imgNums = vecRotMes[0][0].size();
	std::vector<std::vector<std::vector<double>>> vecRotDiffMes(vecMethod.size());
	for (int mNo = 0; mNo < vecMethod.size(); ++mNo) {
		std::vector<std::vector<double>> tmpDiffAngs(vecRotAng.size() - 1);
		for (int aNo = 0; aNo < vecRotAng.size()-1; ++aNo) {
			std::vector<double> tmpDiff(imgNums);
			for (int imgNo = 0; imgNo < imgNums; ++imgNo) {
				tmpDiff[imgNo] = vecRotMes[mNo][aNo][imgNo] - vecRotMes[mNo][vecRotAng.size() - 1][imgNo];
			}
			tmpDiffAngs[aNo] = tmpDiff;
		}
		vecRotDiffMes[mNo] = tmpDiffAngs;
	}
	
	// Calculate the error
	std::vector<std::vector<std::vector<double>>> vecRotErrMes(vecMethod.size());
	for (int mNo = 0; mNo < vecMethod.size(); ++mNo) {
		std::vector<std::vector<double>> tmpDiffAngs(vecRotAng.size() - 1);
		for (int aNo = 0; aNo < vecRotAng.size() - 1; ++aNo) {
			std::vector<double> tmpDiff(imgNums);
			for (int imgNo = 0; imgNo < imgNums; ++imgNo) {
				tmpDiff[imgNo] = vecRotDiffMes[mNo][aNo][imgNo] - vecRotAng[aNo];
			}
			tmpDiffAngs[aNo] = tmpDiff;
		}
		vecRotErrMes[mNo] = tmpDiffAngs;
	}
	
	// Calculate metrics
	std::vector<std::vector<double>> vecRotMAE(vecMethod.size());
	std::vector<std::vector<double>> vecRotMSE(vecMethod.size());
	std::vector<std::vector<double>> vecRotMean(vecMethod.size());
	std::vector<std::vector<double>> vecRotSD(vecMethod.size());
	std::vector<std::vector<double>> vecRotItv(vecMethod.size());
	std::vector<std::vector<double>> vecPassRatio(vecMethod.size());
	std::vector<std::vector<double>> vecRotAvgTime(vecMethod.size());
	for (int mNo = 0; mNo < vecMethod.size(); ++mNo) {
		std::vector<double> vecTmpMAE(vecRotAng.size() - 1);
		std::vector<double> vecTmpMSE(vecRotAng.size() - 1);
		std::vector<double> vecTmpMean(vecRotAng.size() - 1);
		std::vector<double> vecTmpSD(vecRotAng.size() - 1);
		std::vector<double> vecTmpItv(vecRotAng.size() - 1);
		std::vector<double> vecTmpNum(vecRotAng.size() - 1);
		std::vector<double> vecTmpTime(vecRotAng.size() - 1);
		for (int aNo = 0; aNo < vecRotAng.size() - 1; ++aNo) {
			double tmpSumAbs = 0;
			double tmpSumPow = 0;
			double tmpSum = 0;
			double tmpNum = 0;
			double tmpTime = 0;
			for (int imgNo = 0; imgNo < imgNums; ++imgNo) {
				if (vecPassFlag[mNo][aNo][imgNo] > 0) {
					tmpSumAbs += abs(vecRotErrMes[mNo][aNo][imgNo]);
					tmpSumPow += pow(vecRotErrMes[mNo][aNo][imgNo], 2);
					tmpSum += vecRotErrMes[mNo][aNo][imgNo];
					tmpNum += 1;
					tmpTime += vecRotTime[mNo][aNo][imgNo];
				}
			}
			vecTmpMAE[aNo] = tmpSumAbs / tmpNum;
			vecTmpMSE[aNo] = tmpSumPow / tmpNum;
			vecTmpMean[aNo] = tmpSum / tmpNum;
			vecTmpSD[aNo] = std::sqrt(tmpSumPow / (tmpNum - 1));
			vecTmpItv[aNo] = fac * vecTmpSD[aNo] / std::sqrt(tmpNum);
			vecTmpNum[aNo] = double(tmpNum) / double(imgNums);
			vecTmpTime[aNo] = tmpTime / tmpNum;
		}
		vecRotMAE[mNo] = vecTmpMAE;
		vecRotMSE[mNo] = vecTmpMSE;
		vecRotMean[mNo] = vecTmpMean;
		vecRotSD[mNo] = vecTmpSD;
		vecRotItv[mNo] = vecTmpItv;
		vecPassRatio[mNo] = vecTmpNum;
		vecRotAvgTime[mNo] = vecTmpTime;
	}

	for (int aNo = 0; aNo < vecRotAng.size() - 1; ++aNo) {
		std::string resPathRot = csvRoot + "\\rot_" + std::to_string(vecRotAng[aNo]) + ".csv";
		std::ofstream fileRot(resPathRot);
		fileRot << "method,rotMAE,rotMSE,rotMean,rotStd,rotItv,passRatio,time\n";
		for (int mNo = 0; mNo < vecMethod.size(); ++mNo) {
		    fileRot << vecMethod[mNo] << "," << vecRotMAE[mNo][aNo] << "," << vecRotMSE[mNo][aNo] << ","
			<< vecRotMean[mNo][aNo] << "," << vecRotSD[mNo][aNo] << "," << vecRotItv[mNo][aNo] << ","
			<< vecPassRatio[mNo][aNo] << "," << vecRotAvgTime[mNo][aNo] << "\n";
		}
		fileRot.close();
	}


	/*  Translation  */
	// Extracted results
	std::vector<std::vector<std::vector<cv::Point2d>>> vecTraMes(vecMethod.size());
	std::vector<std::vector<std::vector<double>>> vecTraTime(vecMethod.size());
	std::vector<std::vector<std::vector<int>>> vecTraPassFlag(vecMethod.size());
	for (int mNo = 0; mNo < vecMethod.size(); ++mNo) {
		std::vector<std::vector<cv::Point2d>> tmpTras(vecTraOfs.size());
		std::vector<std::vector<double>> tmpTrasTime(vecTraOfs.size());
		std::vector<std::vector<int>> tmpPassFlags(vecTraOfs.size());
		for (int tNo = 0; tNo < vecTraOfs.size(); ++tNo) {
			std::vector<cv::Point2d> tmpImgs;
			std::vector<double> tmpTimes;
		    std::vector<int> tmpFlags;
			std::string filePath = dataRoot + "\\" + vecMethod[mNo] + "\\" + "tra" + "\\" + vecTraFile[tNo] + ".txt";
			std::ifstream file(filePath);
			std::string line;
			while (std::getline(file, line)) {
				std::vector<std::string> items = cm::split(line, " ");
				tmpImgs.push_back(cv::Point2d(std::stod(items[3]), std::stod(items[4])));
				tmpTimes.push_back(std::stod(items[2]));
				tmpFlags.push_back(std::stod(items[1]) > 50 ? 1 : 0);
				if (std::stod(items[1]) < 50) {
					int j = 0;
				}
			}
			tmpTras[tNo] = tmpImgs;
			tmpTrasTime[tNo] = tmpTimes;
			tmpPassFlags[tNo] = tmpFlags;
		}
		vecTraMes[mNo] = tmpTras;
		vecTraTime[mNo] = tmpTrasTime;
		vecTraPassFlag[mNo] = tmpPassFlags;
	}

	// Calculate the distance of relative translation
	std::vector<std::vector<std::vector<cv::Point2d>>> vecTraDiffMes(vecMethod.size());
	for (int mNo = 0; mNo < vecMethod.size(); ++mNo) {
		std::vector<std::vector<cv::Point2d>> tmpTras(vecTraOfs.size()-1);
		for (int tNo = 0; tNo < vecTraOfs.size() - 1; ++tNo) {
			std::vector<cv::Point2d> tmpImgs(imgNums);
			for (int imgNo = 0; imgNo < imgNums; ++imgNo) {
				tmpImgs[imgNo] = vecTraMes[mNo][tNo][imgNo] - vecTraMes[mNo][vecTraOfs.size() - 1][imgNo];
			}
			tmpTras[tNo] = tmpImgs;
		}
		vecTraDiffMes[mNo] = tmpTras;
	}

	// Calculate the error
	std::vector<std::vector<std::vector<cv::Point2d>>> vecTraErrMes(vecMethod.size());
	for (int mNo = 0; mNo < vecMethod.size(); ++mNo) {
		std::vector<std::vector<cv::Point2d>> tmpTras(vecTraOfs.size() - 1);
		for (int tNo = 0; tNo < vecTraOfs.size() - 1; ++tNo) {
			std::vector<cv::Point2d> tmpImgs(imgNums);
			for (int imgNo = 0; imgNo < imgNums; ++imgNo) {
				tmpImgs[imgNo] = vecTraDiffMes[mNo][tNo][imgNo] - vecTraOfs[tNo];
			}
			tmpTras[tNo] = tmpImgs;
		}
		vecTraErrMes[mNo] = tmpTras;
	}
	
	// Calculate metrics
	std::vector<std::vector<double>> vecTraMAE(vecMethod.size());
	std::vector<std::vector<double>> vecTraMSE(vecMethod.size());
	std::vector<std::vector<double>> vecTraMean(vecMethod.size());
	std::vector<std::vector<double>> vecTraSD(vecMethod.size());
	std::vector<std::vector<double>> vecTraItv(vecMethod.size());
	std::vector<std::vector<double>> vecTraPassRatio(vecMethod.size());
	std::vector<std::vector<double>> vecTraAvgTime(vecMethod.size());
	for (int mNo = 0; mNo < vecMethod.size(); ++mNo) {
		std::vector<double> tmpMAEs(vecTraOfs.size() - 1);
		std::vector<double> tmpMSEs(vecTraOfs.size() - 1);
		std::vector<double> tmpMeans(vecTraOfs.size() - 1);
		std::vector<double> tmpSDs(vecTraOfs.size() - 1);
		std::vector<double> tmpItvs(vecTraOfs.size() - 1);
		std::vector<double> tmpPassNums(vecTraOfs.size() - 1);
		std::vector<double> tmpTimes(vecTraOfs.size() - 1);
		for (int tNo = 0; tNo < vecTraOfs.size() - 1; ++tNo) {
			double tmpSumAbs = 0;
			double tmpSumPow = 0;
			double tmpSum = 0;
			double tmpNum = 0;
			double tmpTime = 0;
			for (int imgNo = 0; imgNo < imgNums; ++imgNo) {
				if (vecTraPassFlag[mNo][tNo][imgNo] > 0) {
					tmpSumAbs += abs(vecTraOfs[tNo].x) > 0.1 ? abs(vecTraErrMes[mNo][tNo][imgNo].x) : abs(vecTraErrMes[mNo][tNo][imgNo].y);
					tmpSumPow += abs(vecTraOfs[tNo].x) > 0.1 ? pow(vecTraErrMes[mNo][tNo][imgNo].x, 2) : pow(vecTraErrMes[mNo][tNo][imgNo].y, 2);
					tmpSum += abs(vecTraOfs[tNo].x) > 0.1 ? vecTraErrMes[mNo][tNo][imgNo].x : vecTraErrMes[mNo][tNo][imgNo].y;
					tmpNum += 1;
					tmpTime += vecTraTime[mNo][tNo][imgNo];
				}
			}
			tmpMAEs[tNo] = tmpSumAbs / tmpNum;
			tmpMSEs[tNo] = tmpSumPow / tmpNum;
			tmpMeans[tNo] = tmpSum / tmpNum;
			tmpSDs[tNo] = std::sqrt(tmpSumPow / (tmpNum - 1));
			tmpItvs[tNo] = fac * tmpSDs[tNo] / std::sqrt(tmpNum);
			tmpPassNums[tNo] = double(tmpNum) / double(imgNums);
			tmpTimes[tNo] = tmpTime / tmpNum;
		}
		vecTraMAE[mNo] = tmpMAEs;
		vecTraMSE[mNo] = tmpMSEs;
		vecTraMean[mNo] = tmpMeans;
		vecTraSD[mNo] = tmpSDs;
		vecTraItv[mNo] = tmpItvs;
		vecTraPassRatio[mNo] = tmpPassNums;
		vecTraAvgTime[mNo] = tmpTimes;
	}

	for (int tNo = 0; tNo < vecTraOfs.size() - 1; ++tNo) {
		std::string resPathTra = csvRoot + "\\tra_" + vecTraFile[tNo] + ".csv";
		std::ofstream fileTra(resPathTra);
		fileTra << "method,traMAE,traMSE,traMean,traStd,traItv,passRatio,time\n";
		for (int mNo = 0; mNo < vecMethod.size(); ++mNo)
			fileTra << vecMethod[mNo] << "," << vecTraMAE[mNo][tNo] << "," << vecTraMSE[mNo][tNo] << ","
			<< vecTraMean[mNo][tNo] << "," << vecTraSD[mNo][tNo] << "," << vecTraItv[mNo][tNo] << ","
			<< vecTraPassRatio[mNo][tNo] << "," << vecTraAvgTime[mNo][tNo] << "\n";
		fileTra.close();
	}
	

	/*  Light  */
	const std::vector<std::string> vecTask = { "lig_rect", "lig_qbc" };
	for (std::string taskName : vecTask) {
		// Extracted results
		std::vector<std::vector<std::vector<cv::Point2d>>> vecTraMesL(vecMethod.size());
		std::vector<std::vector<std::vector<double>>> vecRotMesL(vecMethod.size());
		std::vector<std::vector<std::vector<double>>> vecTimeMesL(vecMethod.size());
		std::vector<std::vector<std::vector<int>>> vecPassFlafL(vecMethod.size());
		for (int mNo = 0; mNo < vecMethod.size(); ++mNo) {
			std::vector<std::string> tmpResFile = cm::glob(dataRoot + "\\" + vecMethod[mNo] + "\\" + taskName + "\\*.txt");
			std::vector<std::vector<cv::Point2d>> tmpTraMes(tmpResFile.size());
			std::vector<std::vector<double>> tmpRotMes(tmpResFile.size());
			std::vector<std::vector<double>> tmpTimeMes(tmpResFile.size());
			std::vector<std::vector<int>> tmpPassFlags(tmpResFile.size());
			for (int lNo = 0; lNo < tmpResFile.size(); ++lNo) {
				std::vector<cv::Point2d> tmpTra;
				std::vector<double> tmpRot;
				std::vector<double> tmpTime;
				std::vector<int> tmpPass;
				std::ifstream file(tmpResFile[lNo]);
				std::string line;
				while (std::getline(file, line)) {
					std::vector<std::string> items = cm::split(line, " ");
					tmpTra.push_back(cv::Point2d(std::stod(items[3]), std::stod(items[4])));
					tmpRot.push_back(std::stod(items[5]));
					tmpTime.push_back(std::stod(items[2]));
					tmpPass.push_back(std::stod(items[1]) > 1 ? 1 : 0);
					if (std::stod(items[1]) < 50) {
						int j = 0;
					}
				}
				tmpTraMes[lNo] = tmpTra;
				tmpRotMes[lNo] = tmpRot;
				tmpTimeMes[lNo] = tmpTime;
				tmpPassFlags[lNo] = tmpPass;
			}
			vecTraMesL[mNo] = tmpTraMes;
			vecRotMesL[mNo] = tmpRotMes;
			vecTimeMesL[mNo] = tmpTimeMes;
			vecPassFlafL[mNo] = tmpPassFlags;
		}

		// Calculate the average
		std::vector<std::vector<cv::Point2d>> vecTraMeanL(vecMethod.size());
		std::vector<std::vector<double>> vecRotMeanL(vecMethod.size());
		std::vector<std::vector<double>> vecTimeMeanL(vecMethod.size());
		std::vector<std::vector<int>> vecPassNumL(vecMethod.size());
		for (int mNo = 0; mNo < vecMethod.size(); ++mNo) {
			std::vector<cv::Point2d> tmpTraMean(vecTraMesL[mNo].size());
			std::vector<double> tmpRotMean(vecRotMesL[mNo].size());
			std::vector<double> tmpTimeMean(vecTimeMesL[mNo].size());
			std::vector<int> tmpPassNum(vecPassFlafL[mNo].size());
			for (int lNo = 0; lNo < vecTraMesL[mNo].size(); ++lNo) {
				cv::Point2d tmpTra(0, 0);
				double tmpRot = 0.;
				double tmpTime = 0.;
				int tmpNum = 0;
				for (int iNo = 0; iNo < vecTraMesL[mNo][lNo].size(); ++iNo) {
					if (vecPassFlafL[mNo][lNo][iNo] > 0) {
						tmpTra += vecTraMesL[mNo][lNo][iNo];
						tmpRot += vecRotMesL[mNo][lNo][iNo];
						tmpTime += vecTimeMesL[mNo][lNo][iNo];
						tmpNum += 1;
					}
				}
				tmpTraMean[lNo] = tmpTra / tmpNum;
				tmpRotMean[lNo] = tmpRot / tmpNum;
				tmpTimeMean[lNo] = tmpTime / tmpNum;
				tmpPassNum[lNo] = tmpNum;
			}
			vecTraMeanL[mNo] = tmpTraMean;
			vecRotMeanL[mNo] = tmpRotMean;
			vecTimeMeanL[mNo] = tmpTimeMean;
			vecPassNumL[mNo] = tmpPassNum;
		}

		// Calculate the error
		std::vector<std::vector<std::vector<cv::Point2d>>> vecTraDiffL(vecMethod.size());
		std::vector<std::vector<std::vector<double>>> vecRotDiffL(vecMethod.size());
		for (int mNo = 0; mNo < vecMethod.size(); ++mNo) {
			std::vector<std::vector<cv::Point2d>> tmpTraDiffL(vecTraMesL[mNo].size());
			std::vector<std::vector<double>> tmpRotDiffL(vecRotMesL[mNo].size());
			for (int lNo = 0; lNo < vecTraMesL[mNo].size(); ++lNo) {
				std::vector<cv::Point2d> tmpTra(vecTraMesL[mNo][lNo].size());
				std::vector<double> tmpRot(vecRotMesL[mNo][lNo].size());
				for (int iNo = 0; iNo < vecTraMesL[mNo][lNo].size(); ++iNo) {
					tmpTra[iNo] = vecTraMesL[mNo][lNo][iNo] - vecTraMeanL[mNo][lNo];
					tmpRot[iNo] = vecRotMesL[mNo][lNo][iNo] - vecRotMeanL[mNo][lNo];
				}
				tmpTraDiffL[lNo] = tmpTra;
				tmpRotDiffL[lNo] = tmpRot;
			}
			vecTraDiffL[mNo] = tmpTraDiffL;
			vecRotDiffL[mNo] = tmpRotDiffL;
		}

		// Calculate metrics
		std::vector<double> vecTraMAEL(vecMethod.size());
		std::vector<double> vecTraMSEL(vecMethod.size());
		std::vector<double> vecTraMeannL(vecMethod.size());
		std::vector<double> vecTraSDL(vecMethod.size());
		std::vector<double> vecTraItvL(vecMethod.size());

		std::vector<double> vecRotMAEL(vecMethod.size());
		std::vector<double> vecRotMSEL(vecMethod.size());
		std::vector<double> vecRotMeannL(vecMethod.size());
		std::vector<double> vecRotSDL(vecMethod.size());
		std::vector<double> vecRotItvL(vecMethod.size());

		std::vector<double> vecAvgTimeL(vecMethod.size());
		std::vector<double> vecPassRatioL(vecMethod.size());

		for (int mNo = 0; mNo < vecMethod.size(); ++mNo) {
			double traSumAbs = 0;
			double traSumPow = 0;
			double traSum = 0;
			double rotSumAbs = 0;
			double rotSumPow = 0;
			double rotSum = 0;
			double tmpTime = 0;
			double tmpNum = 0;
			double totalNum = 0;
			for (int lNo = 0; lNo < vecTraMesL[mNo].size(); ++lNo) {
				for (int iNo = 0; iNo < vecTraMesL[mNo][lNo].size(); ++iNo) {
					if (vecPassFlafL[mNo][lNo][iNo] > 0) {
						tmpNum += 1;
						const double tmpTraVal = abs(vecTraDiffL[mNo][lNo][iNo].x) > abs(vecTraDiffL[mNo][lNo][iNo].y) ? vecTraDiffL[mNo][lNo][iNo].x : vecTraDiffL[mNo][lNo][iNo].y;

						traSumAbs += abs(tmpTraVal);
						traSumPow += pow(tmpTraVal, 2);
						traSum += tmpTraVal;

						rotSumAbs += abs(vecRotDiffL[mNo][lNo][iNo]);
						rotSumPow += pow(vecRotDiffL[mNo][lNo][iNo], 2);
						rotSum += vecRotDiffL[mNo][lNo][iNo];

						tmpTime += vecTimeMesL[mNo][lNo][iNo];
					}
				}
				totalNum += vecTraDiffL[mNo][lNo].size();
			}
			vecTraMAEL[mNo] = traSumAbs / tmpNum;
			vecTraMSEL[mNo] = traSumPow / tmpNum;
			vecTraMeannL[mNo] = traSum / tmpNum;
			vecTraSDL[mNo] = std::sqrt(traSumPow / (tmpNum - 1));
			vecTraItvL[mNo] = fac * vecTraSDL[mNo] / std::sqrt(tmpNum);

			vecRotMAEL[mNo] = rotSumAbs / tmpNum;
			vecRotMSEL[mNo] = rotSumPow / tmpNum;
			vecRotMeannL[mNo] = rotSum / tmpNum;
			vecRotSDL[mNo] = std::sqrt(rotSumPow / (tmpNum - 1));
			vecRotItvL[mNo] = fac * vecRotSDL[mNo] / std::sqrt(tmpNum);

			vecAvgTimeL[mNo] = tmpTime / tmpNum;
			vecPassRatioL[mNo] = tmpNum / totalNum;
		}
		std::string resPathLig = csvRoot + "\\" + taskName + ".csv";
		std::ofstream fileLig(resPathLig);
		fileLig << "method,traMAE,traMSE,traMean,traStd,traItv,rotMAE,rotMSE,rotMean,rotStd,rotItv,passRatio,time\n";
		for (int mNo = 0; mNo < vecMethod.size(); ++mNo)
			fileLig << vecMethod[mNo] << ","
			<< vecTraMAEL[mNo] << "," << vecTraMSEL[mNo] << "," << vecTraMeannL[mNo] << "," << vecTraSDL[mNo] << "," << vecTraItvL[mNo] << ","
			<< vecRotMAEL[mNo] << "," << vecRotMSEL[mNo] << "," << vecRotMeannL[mNo] << "," << vecRotSDL[mNo] << "," << vecRotItvL[mNo] << ","
			<< vecPassRatioL[mNo] << "," << vecAvgTimeL[mNo] << "\n";
		fileLig.close();
	}

	return 0;
}
