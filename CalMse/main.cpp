#include <fstream>
#include <filesystem>
#include <opencv2/opencv.hpp>
#include "cm_file.hpp"


// Aggregate benchmark result files and export accuracy metrics as CSV.
int main() {
	const cv::Size imgSize(512, 512);
	const std::string dataRoot = "..\\res";
	std::vector<std::string> vecMethod = cm::glob(dataRoot + "\\*");  // Result method folders to summarize.
	const std::vector<double> vecRotAng = { -10, -5, 5, 10, 0 };  // Tested rotation angles, with 0 as baseline.
	const std::vector<std::string> vecRotFile = { "-10", "-5", "5", "10", "0" };  // Rotation result file suffixes.
	const std::vector<cv::Point2d> vecTraOfs = { cv::Point2d(0, -10), cv::Point2d(0, 10), cv::Point2d(-10, 0), cv::Point2d(10, 0), cv::Point2d(0, 0) };  // Tested translation offsets, with (0,0) as baseline.
	const std::vector<std::string> vecTraFile = { "up", "down", "left", "right", "mid" };  // Translation result file suffixes.
	const std::string csvRoot = "..\\csv";  // Output directory for generated CSV summaries.
	if (!std::filesystem::exists(csvRoot)) {
		std::filesystem::create_directories(csvRoot);
	}

	/*  Rotation  */
	// Extracted results
	std::vector<std::vector<std::vector<double>>> vecRotMes(vecMethod.size());  // Raw measured angles by method, angle, and image.
	std::vector<std::vector<std::vector<double>>> vecRotTime(vecMethod.size());  // Raw rotation inference times by method, angle, and image.
	std::vector<std::vector<std::vector<int>>> vecPassFlag(vecMethod.size());  // Rotation pass flags by method, angle, and image.
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
				tmpFlags.push_back(std::stod(items[1]) > 51 ? 1 : 0);
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
	std::vector<std::vector<std::vector<double>>> vecRotDiffMes(vecMethod.size());  // Angle offsets relative to the zero-rotation baseline.
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
	
	// Calculate the average
	std::vector<std::vector<double>> vecRotMean(vecMethod.size());  // Mean measured rotation offset per method and angle.
	for (int mNo = 0; mNo < vecMethod.size(); ++mNo) {
		std::vector<double> tmpDiffAngs(vecRotAng.size() - 1);
		for (int aNo = 0; aNo < vecRotAng.size() - 1; ++aNo) {
			double tmpRotDiffSum = 0;
			double tmpRotNum = 0;
			for (int imgNo = 0; imgNo < imgNums; ++imgNo) {
				if (vecPassFlag[mNo][aNo][imgNo] > 0) {
					tmpRotDiffSum += vecRotDiffMes[mNo][aNo][imgNo];
					tmpRotNum += 1;
				}
			}
			tmpDiffAngs[aNo] = tmpRotDiffSum / tmpRotNum;
		}
		vecRotMean[mNo] = tmpDiffAngs;
	}
	
	// Calculate metrics
	std::vector<std::vector<double>> vecRotMAE(vecMethod.size());  // Rotation mean absolute error per method and angle.
	std::vector<std::vector<double>> vecRotSD(vecMethod.size());  // Rotation standard deviation per method and angle.
	std::vector<std::vector<double>> vecRotSU(vecMethod.size());  // Rotation standard uncertainty per method and angle.
	std::vector<std::vector<double>> vecPassRatio(vecMethod.size());  // Rotation valid-result ratio per method and angle.
	std::vector<std::vector<double>> vecRotAvgTime(vecMethod.size());  // Average rotation inference time per method and angle.
	for (int mNo = 0; mNo < vecMethod.size(); ++mNo) {
		std::vector<double> vecTmpMAE(vecRotAng.size() - 1);
		std::vector<double> vecTmpSD(vecRotAng.size() - 1);
		std::vector<double> vecTmpSU(vecRotAng.size() - 1);
		std::vector<double> vecTmpNum(vecRotAng.size() - 1);
		std::vector<double> vecTmpTime(vecRotAng.size() - 1);
		for (int aNo = 0; aNo < vecRotAng.size() - 1; ++aNo) {
			double tmpSumAbs = 0;
			double tmpSumPow = 0;
			double tmpNum = 0;
			double tmpTime = 0;
			for (int imgNo = 0; imgNo < imgNums; ++imgNo) {
				if (vecPassFlag[mNo][aNo][imgNo] > 0) {
					tmpSumAbs += abs(vecRotDiffMes[mNo][aNo][imgNo] - vecRotMean[mNo][aNo]);
					tmpSumPow += pow(vecRotDiffMes[mNo][aNo][imgNo] - vecRotMean[mNo][aNo], 2);
					tmpNum += 1;
					tmpTime += vecRotTime[mNo][aNo][imgNo];
				}
			}
			vecTmpMAE[aNo] = tmpSumAbs / tmpNum;
			vecTmpSD[aNo] = std::sqrt(tmpSumPow / (tmpNum - 1));
			vecTmpSU[aNo] = vecTmpSD[aNo] / std::sqrt(tmpNum);
			vecTmpNum[aNo] = double(tmpNum) / double(imgNums);
			vecTmpTime[aNo] = tmpTime / tmpNum;
		}
		vecRotMAE[mNo] = vecTmpMAE;
		vecRotSD[mNo] = vecTmpSD;
		vecRotSU[mNo] = vecTmpSU;
		vecPassRatio[mNo] = vecTmpNum;
		vecRotAvgTime[mNo] = vecTmpTime;
	}

	// Write one rotation summary file per tested angle.
	for (int aNo = 0; aNo < vecRotAng.size() - 1; ++aNo) {
		std::string resPathRot = csvRoot + "\\rot_" + std::to_string(vecRotAng[aNo]) + ".csv";  // Rotation metric CSV path.
		std::ofstream fileRot(resPathRot);  // Rotation metric CSV writer.
		fileRot << "method,rotMean,rotMAE,rotSD,rotSU,passRatio,time\n";
		for (int mNo = 0; mNo < vecMethod.size(); ++mNo) {
		    fileRot 
				<< vecMethod[mNo] << "," 
			    << vecRotMean[mNo][aNo] << "," << vecRotMAE[mNo][aNo] << "," 
			    << vecRotSD[mNo][aNo] << "," << vecRotSU[mNo][aNo] << "," 
			    << vecPassRatio[mNo][aNo] << "," << vecRotAvgTime[mNo][aNo] << "\n";
		}
		fileRot.close();
	}


	/*  Translation  */
	// Extracted results
	std::vector<std::vector<std::vector<cv::Point2d>>> vecTraMes(vecMethod.size());  // Raw measured offsets by method, direction, and image.
	std::vector<std::vector<std::vector<double>>> vecTraTime(vecMethod.size());  // Raw translation inference times by method, direction, and image.
	std::vector<std::vector<std::vector<int>>> vecTraPassFlag(vecMethod.size());  // Translation pass flags by method, direction, and image.
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
				tmpFlags.push_back(std::stod(items[1]) > 51 ? 1 : 0);
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
	std::vector<std::vector<std::vector<cv::Point2d>>> vecTraDiffMes(vecMethod.size());  // Translation offsets relative to the zero-offset baseline.
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

	// Calculate the average
	std::vector<std::vector<cv::Point2d>> vecTraPtMean(vecMethod.size());  // Mean measured 2D translation offset per method and direction.
	for (int mNo = 0; mNo < vecMethod.size(); ++mNo) {
		std::vector<cv::Point2d> tmpTras(vecTraOfs.size() - 1);
		for (int tNo = 0; tNo < vecTraOfs.size() - 1; ++tNo) {
			cv::Point2d tmpTraSum = 0;
			double tmpNum = 0;
			for (int imgNo = 0; imgNo < imgNums; ++imgNo) {
				if (vecTraPassFlag[mNo][tNo][imgNo] > 0) {
					tmpTraSum += vecTraDiffMes[mNo][tNo][imgNo];
					tmpNum += 1;
				}
			}
			tmpTras[tNo] = cv::Point2d(tmpTraSum.x / tmpNum, tmpTraSum.y / tmpNum);
		}
		vecTraPtMean[mNo] = tmpTras;
	}
	
	// Calculate metrics
	std::vector<std::vector<double>> vecTraMean(vecMethod.size());  // Axis-aligned mean translation per method and direction.
	std::vector<std::vector<double>> vecTraMAE(vecMethod.size());  // Translation mean absolute error per method and direction.
	std::vector<std::vector<double>> vecTraSD(vecMethod.size());  // Translation standard deviation per method and direction.
	std::vector<std::vector<double>> vecTraSU(vecMethod.size());  // Translation standard uncertainty per method and direction.
	std::vector<std::vector<double>> vecTraPassRatio(vecMethod.size());  // Translation valid-result ratio per method and direction.
	std::vector<std::vector<double>> vecTraAvgTime(vecMethod.size());  // Average translation inference time per method and direction.
	for (int mNo = 0; mNo < vecMethod.size(); ++mNo) {
		std::vector<double> tmpMeans(vecTraOfs.size() - 1);
		std::vector<double> tmpMAEs(vecTraOfs.size() - 1);
		std::vector<double> tmpSDs(vecTraOfs.size() - 1);
		std::vector<double> tmpSUs(vecTraOfs.size() - 1);
		std::vector<double> tmpPassNums(vecTraOfs.size() - 1);
		std::vector<double> tmpTimes(vecTraOfs.size() - 1);
		for (int tNo = 0; tNo < vecTraOfs.size() - 1; ++tNo) {
			double tmpSumAbs = 0;
			double tmpSumPow = 0;
			double tmpNum = 0;
			double tmpTime = 0;
			for (int imgNo = 0; imgNo < imgNums; ++imgNo) {
				if (vecTraPassFlag[mNo][tNo][imgNo] > 0) {
					tmpSumAbs += abs(vecTraOfs[tNo].x) > 0.1 ? abs(vecTraDiffMes[mNo][tNo][imgNo].x - vecTraPtMean[mNo][tNo].x) : abs(vecTraDiffMes[mNo][tNo][imgNo].y - vecTraPtMean[mNo][tNo].y);
					tmpSumPow += abs(vecTraOfs[tNo].x) > 0.1 ? pow(vecTraDiffMes[mNo][tNo][imgNo].x - vecTraPtMean[mNo][tNo].x, 2) : pow(vecTraDiffMes[mNo][tNo][imgNo].y - vecTraPtMean[mNo][tNo].y, 2);
					tmpNum += 1;
					tmpTime += vecTraTime[mNo][tNo][imgNo];
				}
			}
			tmpMeans[tNo] = abs(vecTraOfs[tNo].x) > 0.1 ? vecTraPtMean[mNo][tNo].x : vecTraPtMean[mNo][tNo].y;
			tmpMAEs[tNo] = tmpSumAbs / tmpNum;
			tmpSDs[tNo] = std::sqrt(tmpSumPow / (tmpNum - 1));
			tmpSUs[tNo] = tmpSDs[tNo] / std::sqrt(tmpNum);
			tmpPassNums[tNo] = double(tmpNum) / double(imgNums);
			tmpTimes[tNo] = tmpTime / tmpNum;
		}
		vecTraMean[mNo] = tmpMeans;
		vecTraMAE[mNo] = tmpMAEs;
		vecTraSD[mNo] = tmpSDs;
		vecTraSU[mNo] = tmpSUs;
		vecTraPassRatio[mNo] = tmpPassNums;
		vecTraAvgTime[mNo] = tmpTimes;
	}

	// Write one translation summary file per tested direction.
	for (int tNo = 0; tNo < vecTraOfs.size() - 1; ++tNo) {
		std::string resPathTra = csvRoot + "\\tra_" + vecTraFile[tNo] + ".csv";  // Translation metric CSV path.
		std::ofstream fileTra(resPathTra);  // Translation metric CSV writer.
		fileTra << "method,traMean,traMAE,traSD,traSU,passRatio,time\n";
		for (int mNo = 0; mNo < vecMethod.size(); ++mNo)
			fileTra << vecMethod[mNo] << "," 
			<< vecTraMean[mNo][tNo] << "," << vecTraMAE[mNo][tNo] << ","
			<< vecTraSD[mNo][tNo] << "," << vecTraSU[mNo][tNo] << "," 
			<< vecTraPassRatio[mNo][tNo] << "," << vecTraAvgTime[mNo][tNo] << "\n";
		fileTra.close();
	}
	

	/*  Light  */
	// Summarize light-condition robustness tasks.
	const std::vector<std::string> vecTask = { "lig_rect", "lig_qb" };
	for (std::string taskName : vecTask) {
		// Extracted results
		std::vector<std::vector<std::vector<cv::Point2d>>> vecTraMesL(vecMethod.size());  // Light-task measured offsets by method, file, and image.
		std::vector<std::vector<std::vector<double>>> vecRotMesL(vecMethod.size());  // Light-task measured angles by method, file, and image.
		std::vector<std::vector<std::vector<double>>> vecTimeMesL(vecMethod.size());  // Light-task inference times by method, file, and image.
		std::vector<std::vector<std::vector<int>>> vecPassFlagL(vecMethod.size());  // Light-task pass flags by method, file, and image.
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
					tmpPass.push_back(std::stod(items[1]) > 51 ? 1 : 0);
				}
				tmpTraMes[lNo] = tmpTra;
				tmpRotMes[lNo] = tmpRot;
				tmpTimeMes[lNo] = tmpTime;
				tmpPassFlags[lNo] = tmpPass;
			}
			vecTraMesL[mNo] = tmpTraMes;
			vecRotMesL[mNo] = tmpRotMes;
			vecTimeMesL[mNo] = tmpTimeMes;
			vecPassFlagL[mNo] = tmpPassFlags;
		}

		// Calculate the average
		std::vector<std::vector<cv::Point2d>> vecTraMeanL(vecMethod.size());  // Light-task mean offset per method and file.
		std::vector<std::vector<double>> vecRotMeanL(vecMethod.size());  // Light-task mean angle per method and file.
		std::vector<std::vector<double>> vecTimeMeanL(vecMethod.size());  // Light-task mean time per method and file.
		std::vector<std::vector<int>> vecPassNumL(vecMethod.size());  // Light-task valid-result count per method and file.
		for (int mNo = 0; mNo < vecMethod.size(); ++mNo) {
			std::vector<cv::Point2d> tmpTraMean(vecTraMesL[mNo].size());
			std::vector<double> tmpRotMean(vecRotMesL[mNo].size());
			std::vector<double> tmpTimeMean(vecTimeMesL[mNo].size());
			std::vector<int> tmpPassNum(vecPassFlagL[mNo].size());
			for (int lNo = 0; lNo < vecTraMesL[mNo].size(); ++lNo) {
				cv::Point2d tmpTra(0, 0);
				double tmpRot = 0.;
				double tmpTime = 0.;
				int tmpNum = 0;
				for (int iNo = 0; iNo < vecTraMesL[mNo][lNo].size(); ++iNo) {
					if (vecPassFlagL[mNo][lNo][iNo] > 0) {
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
		std::vector<std::vector<std::vector<cv::Point2d>>> vecTraDiffL(vecMethod.size());  // Light-task translation residuals after mean removal.
		std::vector<std::vector<std::vector<double>>> vecRotDiffL(vecMethod.size());  // Light-task rotation residuals after mean removal.
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
		std::vector<double> vecTraMAEL(vecMethod.size());  // Light-task aggregate translation MAE per method.
		std::vector<double> vecTraSDL(vecMethod.size());  // Light-task aggregate translation SD per method.
		std::vector<double> vecTraSUL(vecMethod.size());  // Light-task aggregate translation SU per method.

		std::vector<double> vecRotMAEL(vecMethod.size());  // Light-task aggregate rotation MAE per method.
		std::vector<double> vecRotSDL(vecMethod.size());  // Light-task aggregate rotation SD per method.
		std::vector<double> vecRotSUL(vecMethod.size());  // Light-task aggregate rotation SU per method.

		std::vector<double> vecAvgTimeL(vecMethod.size());  // Light-task aggregate average time per method.
		std::vector<double> vecPassRatioL(vecMethod.size());  // Light-task aggregate valid-result ratio per method.

		for (int mNo = 0; mNo < vecMethod.size(); ++mNo) {
			double traSumAbs = 0;
			double traSumPow = 0;
			double rotSumAbs = 0;
			double rotSumPow = 0;
			double tmpTime = 0;
			double tmpNum = 0;
			double totalNum = 0;
			for (int lNo = 0; lNo < vecTraMesL[mNo].size(); ++lNo) {
				for (int iNo = 0; iNo < vecTraMesL[mNo][lNo].size(); ++iNo) {
					if (vecPassFlagL[mNo][lNo][iNo] > 0) {
						tmpNum += 1;
						//const double tmpTraVal = abs(vecTraDiffL[mNo][lNo][iNo].x) > abs(vecTraDiffL[mNo][lNo][iNo].y) ? vecTraDiffL[mNo][lNo][iNo].x : vecTraDiffL[mNo][lNo][iNo].y;
						const double tmpTraVal = pow(vecTraDiffL[mNo][lNo][iNo].x, 2) + pow(vecTraDiffL[mNo][lNo][iNo].y, 2);

						traSumAbs += std::sqrt(tmpTraVal);
						traSumPow += tmpTraVal;

						rotSumAbs += abs(vecRotDiffL[mNo][lNo][iNo]);
						rotSumPow += pow(vecRotDiffL[mNo][lNo][iNo], 2);

						tmpTime += vecTimeMesL[mNo][lNo][iNo];
					}
				}
				totalNum += vecTraDiffL[mNo][lNo].size();
			}
			vecTraMAEL[mNo] = traSumAbs / tmpNum;
			vecTraSDL[mNo] = std::sqrt(traSumPow / (tmpNum - 1));
			vecTraSUL[mNo] = vecTraSDL[mNo] / std::sqrt(tmpNum);

			vecRotMAEL[mNo] = rotSumAbs / tmpNum;
			vecRotSDL[mNo] = std::sqrt(rotSumPow / (tmpNum - 1));
			vecRotSUL[mNo] = vecRotSDL[mNo] / std::sqrt(tmpNum);

			vecAvgTimeL[mNo] = tmpTime / tmpNum;
			vecPassRatioL[mNo] = tmpNum / totalNum;
		}
		// Write the aggregate light-condition metrics.
		std::string resPathLig = csvRoot + "\\" + taskName + ".csv";  // Light-task metric CSV path.
		std::ofstream fileLig(resPathLig);  // Light-task metric CSV writer.
		fileLig << "method,traMAE,traSD,traSU,rotMAE,rotSD,rotSU,passRatio,time\n";
		for (int mNo = 0; mNo < vecMethod.size(); ++mNo)
			fileLig << vecMethod[mNo] << ","
			<< vecTraMAEL[mNo] << "," << vecTraSDL[mNo] << "," << vecTraSUL[mNo] << ","
			<< vecRotMAEL[mNo] << "," << vecRotSDL[mNo] << "," << vecRotSUL[mNo] << ","
			<< vecPassRatioL[mNo] << "," << vecAvgTimeL[mNo] << "\n";
		fileLig.close();
	}

	return 0;
}
