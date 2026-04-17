#pragma once
#ifndef CM_INTRINSICS_HPP
#define CM_INTRINSICS_HPP
#include <opencv2/opencv.hpp>
#ifdef __AVX2__
#include <immintrin.h>
#endif


namespace cm {
	 inline void getNormalizedGradientAndMagnitudeImages(const cv::Mat& inSrcImg, std::vector<cv::Mat>& outGradImgs, cv::Mat& outMagImg, const bool& inGetMagFlag, float* outMaxMagPtr = nullptr, const int& inSobelSize = 3) {
#ifndef __AVX2__
		cv::Mat gradRoiImgX = cv::Mat(inSrcImg.size(), CV_32FC1);
		cv::Mat gradRoiImgY = cv::Mat(inSrcImg.size(), CV_32FC1);
		cv::Sobel(inSrcImg, gradRoiImgX, CV_32F, 1, 0, inSobelSize);
		cv::Sobel(inSrcImg, gradRoiImgY, CV_32F, 0, 1, inSobelSize);
		cv::Mat magnitudeImg;
		cv::magnitude(gradRoiImgX, gradRoiImgY, magnitudeImg);
		magnitudeImg += 1e-6;
		cv::Mat gradNorRoiImgX = gradRoiImgX / magnitudeImg;
		cv::Mat gradNorRoiImgY = gradRoiImgY / magnitudeImg;
		outGradImgs = { gradNorRoiImgX, gradNorRoiImgY };
		if (inGetMagFlag) {
			cv::normalize(magnitudeImg, outMagImg, 1, 0, cv::NORM_MINMAX, CV_32FC1);
		}
		if (outMaxMagPtr != nullptr) {
			double minMag, maxMag;
			cv::minMaxLoc(magnitudeImg, &minMag, &maxMag);
			*outMaxMagPtr = maxMag;
		}
#else
		outGradImgs.resize(2);
		outGradImgs[0] = cv::Mat(inSrcImg.size(), CV_32FC1);
		outGradImgs[1] = cv::Mat(inSrcImg.size(), CV_32FC1);
		outMagImg = cv::Mat(inSrcImg.size(), CV_32FC1);
		float* pGradX = (float*)outGradImgs[0].data;
		float* pGradY = (float*)outGradImgs[1].data;
		float* pMag = (float*)outMagImg.data;

		const int loopNum = ceilf((inSrcImg.cols) / (float)16);
		const int restNum = inSrcImg.cols % 16;
		const int blockSize = loopNum * 16 + 2;
		cv::Mat buffer = cv::Mat(cv::Size(blockSize * 3, 1), CV_8UC1);
		uchar* pBuffer = (uchar*)buffer.data;
		uchar* pSrcImg = inSrcImg.data;

		if (pBuffer == NULL) {
			return;
		}

		uchar* pLine0 = pBuffer;
		uchar* pLine1 = pBuffer + blockSize;
		uchar* pLine2 = pBuffer + blockSize * 2;

		memset(pLine0, *pSrcImg, 1);
		memcpy(pLine0 + 1, pSrcImg, inSrcImg.cols);
		memset(pLine0 + 1 + inSrcImg.cols, *(pSrcImg + inSrcImg.cols - 1), blockSize - inSrcImg.cols - 1);
		memcpy(pLine1, pLine0, blockSize);

		__m256i vecImgVal[8];
		__m256i GX, GY;
		float vecRestGX[16];
		float vecRestGY[16];
		float vecRestMag[16];
		__m256 GX0, GX1, GY0, GY1;
		__m256 Mag0, Mag1;
		__m256 fMag0, fMag1;
		__m256 tmpMax0 = _mm256_setzero_ps(), tmpMax1 = _mm256_setzero_ps();
		__m256 delta = _mm256_set1_ps(1e-6);
		int idx;
		for (int row = 0; row < inSrcImg.rows; ++row) {
			pLine0 = pBuffer + (row % 3) * blockSize;
			pLine1 = pBuffer + ((row + 1) % 3) * blockSize;
			pLine2 = pBuffer + ((row + 2) % 3) * blockSize;
			if (row != inSrcImg.rows - 1) {
				memset(pLine2, *(pSrcImg + (row + 1) * inSrcImg.cols), 1);
				memcpy(pLine2 + 1, pSrcImg + (row + 1) * inSrcImg.cols, inSrcImg.cols);
				memset(pLine2 + 1 + inSrcImg.cols, *(pSrcImg + (row + 2) * inSrcImg.cols - 1), blockSize - inSrcImg.cols - 1);
			}
			else {
				memcpy(pLine2, pLine1, blockSize);
			}

			for (int loopNo = 0; loopNo < loopNum; ++loopNo) {
				vecImgVal[0] = _mm256_cvtepu8_epi16(_mm_load_si128((__m128i*)(pLine0 + loopNo * 16)));
				vecImgVal[1] = _mm256_cvtepu8_epi16(_mm_load_si128((__m128i*)(pLine0 + loopNo * 16 + 1)));
				vecImgVal[2] = _mm256_cvtepu8_epi16(_mm_load_si128((__m128i*)(pLine0 + loopNo * 16 + 2)));
				vecImgVal[3] = _mm256_cvtepu8_epi16(_mm_load_si128((__m128i*)(pLine1 + loopNo * 16)));
				vecImgVal[4] = _mm256_cvtepu8_epi16(_mm_load_si128((__m128i*)(pLine1 + loopNo * 16 + 2)));
				vecImgVal[5] = _mm256_cvtepu8_epi16(_mm_load_si128((__m128i*)(pLine2 + loopNo * 16)));
				vecImgVal[6] = _mm256_cvtepu8_epi16(_mm_load_si128((__m128i*)(pLine2 + loopNo * 16 + 1)));
				vecImgVal[7] = _mm256_cvtepu8_epi16(_mm_load_si128((__m128i*)(pLine2 + loopNo * 16 + 2)));

				GX = _mm256_sub_epi16(_mm256_add_epi16(_mm256_add_epi16(vecImgVal[2], vecImgVal[7]), _mm256_slli_epi16(vecImgVal[4], 1)),
					_mm256_add_epi16(_mm256_add_epi16(vecImgVal[0], vecImgVal[5]), _mm256_slli_epi16(vecImgVal[3], 1)));
				GY = _mm256_sub_epi16(_mm256_add_epi16(_mm256_add_epi16(vecImgVal[5], vecImgVal[7]), _mm256_slli_epi16(vecImgVal[6], 1)),
					_mm256_add_epi16(_mm256_add_epi16(vecImgVal[0], vecImgVal[2]), _mm256_slli_epi16(vecImgVal[1], 1)));

				GX0 = _mm256_cvtepi32_ps(_mm256_cvtepi16_epi32(_mm256_castsi256_si128(GX)));
				GX1 = _mm256_cvtepi32_ps(_mm256_cvtepi16_epi32(_mm256_extracti128_si256(GX, 1)));
				GY0 = _mm256_cvtepi32_ps(_mm256_cvtepi16_epi32(_mm256_castsi256_si128(GY)));
				GY1 = _mm256_cvtepi32_ps(_mm256_cvtepi16_epi32(_mm256_extracti128_si256(GY, 1)));

				Mag0 = _mm256_sqrt_ps(_mm256_add_ps(_mm256_mul_ps(GX0, GX0), _mm256_mul_ps(GY0, GY0)));
				Mag1 = _mm256_sqrt_ps(_mm256_add_ps(_mm256_mul_ps(GX1, GX1), _mm256_mul_ps(GY1, GY1)));

				if (inGetMagFlag) {
					tmpMax0 = _mm256_max_ps(tmpMax0, Mag0);
					tmpMax1 = _mm256_max_ps(tmpMax1, Mag1);
				}

				fMag0 = _mm256_rcp_ps(_mm256_add_ps(Mag0, delta));
				fMag1 = _mm256_rcp_ps(_mm256_add_ps(Mag1, delta));

				GX0 = _mm256_mul_ps(GX0, fMag0);
				GX1 = _mm256_mul_ps(GX1, fMag1);
				GY0 = _mm256_mul_ps(GY0, fMag0);
				GY1 = _mm256_mul_ps(GY1, fMag1);

				idx = row * inSrcImg.cols + 16 * loopNo;
				if (restNum != 0 && loopNo == loopNum - 1) {
					_mm256_store_ps(vecRestGX, GX0);
					_mm256_store_ps(vecRestGX + 8, GX1);
					_mm256_store_ps(vecRestGY, GY0);
					_mm256_store_ps(vecRestGY + 8, GY1);
					for (int restNo = 0; restNo < restNum; ++restNo) {
						*(pGradX + idx + restNo) = vecRestGX[restNo];
						*(pGradY + idx + restNo) = vecRestGY[restNo];
					}
					if (inGetMagFlag) {
						_mm256_store_ps(vecRestMag, Mag0);
						_mm256_store_ps(vecRestMag + 8, Mag1);
						for (int restNo = 0; restNo < restNum; ++restNo) {
							*(pMag + idx + restNo) = vecRestMag[restNo];
						}
					}
				}
				else {
					_mm256_store_ps(pGradX + idx, GX0);
					_mm256_store_ps(pGradX + idx + 8, GX1);
					_mm256_store_ps(pGradY + idx, GY0);
					_mm256_store_ps(pGradY + idx + 8, GY1);
					if (inGetMagFlag) {
						_mm256_store_ps(pMag + idx, Mag0);
						_mm256_store_ps(pMag + idx + 8, Mag1);
					}
				}
			}
		}

		float maxMag = 0;
		if (inGetMagFlag) {
			__m256 tmpMax = _mm256_max_ps(tmpMax0, tmpMax1);
			float vecTmpMax[8];
			_mm256_store_ps(vecTmpMax, tmpMax);
			for (int i = 0; i < 8; ++i) {
				if (vecTmpMax[i] > maxMag) maxMag = vecTmpMax[i];
			}
			float fMag = 1. / (maxMag + 1e-6);
			const int magLoopNum = outMagImg.total() / 8;
			__m256 mag;
			__m256 f = _mm256_set1_ps(fMag);
			for (int magLoopNo = 0; magLoopNo < magLoopNum; ++magLoopNo) {
				mag = _mm256_load_ps(pMag + magLoopNo * 8);
				mag = _mm256_mul_ps(mag, f);
				_mm256_store_ps(pMag + magLoopNo * 8, mag);
			}
			const int magRestNum = outMagImg.total() % 8;
			int beginIdx = magLoopNum * 8;
			for (int magRestNo = 0; magRestNo < magRestNum; ++magRestNo) {
				*(pMag + magLoopNum * 8 + magRestNo) = *(pMag + magLoopNum * 8 + magRestNo) * fMag;
			}
		}

		if (outMaxMagPtr != nullptr) {
			*outMaxMagPtr = maxMag;
		}
#endif
	}

	 inline void getNormalizedMagnitudeImages(const cv::Mat& inSrcImg, cv::Mat& outMagImg, float* outMaxMagPtr = nullptr, const int& inSobelSize = 3) {
#ifndef __AVX2__
		cv::Mat gradRoiImgX = cv::Mat(inSrcImg.size(), CV_32FC1);
		cv::Mat gradRoiImgY = cv::Mat(inSrcImg.size(), CV_32FC1);
		cv::Sobel(inSrcImg, gradRoiImgX, CV_32F, 1, 0, inSobelSize);
		cv::Sobel(inSrcImg, gradRoiImgY, CV_32F, 0, 1, inSobelSize);
		cv::Mat magnitudeImg;
		cv::magnitude(gradRoiImgX, gradRoiImgY, magnitudeImg);
		magnitudeImg += 1e-6;
		cv::Mat gradNorRoiImgX = gradRoiImgX / magnitudeImg;
		cv::Mat gradNorRoiImgY = gradRoiImgY / magnitudeImg;
		outGradImgs = { gradNorRoiImgX, gradNorRoiImgY };
		if (inGetMagFlag) {
			cv::normalize(magnitudeImg, outMagImg, 1, 0, cv::NORM_MINMAX, CV_32FC1);
		}
		if (outMaxMagPtr != nullptr) {
			double minMag, maxMag;
			cv::minMaxLoc(magnitudeImg, &minMag, &maxMag);
			*outMaxMagPtr = maxMag;
		}
#else
		outMagImg = cv::Mat(inSrcImg.size(), CV_32FC1);
		float* pMag = (float*)outMagImg.data;

		const int loopNum = ceilf((inSrcImg.cols) / (float)16);
		const int restNum = inSrcImg.cols % 16;
		const int blockSize = loopNum * 16 + 2;
		cv::Mat buffer = cv::Mat(cv::Size(blockSize * 3, 1), CV_8UC1);
		uchar* pBuffer = (uchar*)buffer.data;
		uchar* pSrcImg = inSrcImg.data;

		if (pBuffer == NULL) {
			return;
		}

		uchar* pLine0 = pBuffer;
		uchar* pLine1 = pBuffer + blockSize;
		uchar* pLine2 = pBuffer + blockSize * 2;

		memset(pLine0, *pSrcImg, 1);
		memcpy(pLine0 + 1, pSrcImg, inSrcImg.cols);
		memset(pLine0 + 1 + inSrcImg.cols, *(pSrcImg + inSrcImg.cols - 1), blockSize - inSrcImg.cols - 1);
		memcpy(pLine1, pLine0, blockSize);

		__m256i vecImgVal[8];
		__m256i GX, GY;
		float vecRestGX[16];
		float vecRestGY[16];
		float vecRestMag[16];
		__m256 GX0, GX1, GY0, GY1;
		__m256 Mag0, Mag1;
		__m256 fMag0, fMag1;
		__m256 tmpMax0 = _mm256_setzero_ps(), tmpMax1 = _mm256_setzero_ps();
		__m256 delta = _mm256_set1_ps(1e-6);
		int idx;
		for (int row = 0; row < inSrcImg.rows; ++row) {
			pLine0 = pBuffer + (row % 3) * blockSize;
			pLine1 = pBuffer + ((row + 1) % 3) * blockSize;
			pLine2 = pBuffer + ((row + 2) % 3) * blockSize;
			if (row != inSrcImg.rows - 1) {
				memset(pLine2, *(pSrcImg + (row + 1) * inSrcImg.cols), 1);
				memcpy(pLine2 + 1, pSrcImg + (row + 1) * inSrcImg.cols, inSrcImg.cols);
				memset(pLine2 + 1 + inSrcImg.cols, *(pSrcImg + (row + 2) * inSrcImg.cols - 1), blockSize - inSrcImg.cols - 1);
			}
			else {
				memcpy(pLine2, pLine1, blockSize);
			}

			for (int loopNo = 0; loopNo < loopNum; ++loopNo) {
				vecImgVal[0] = _mm256_cvtepu8_epi16(_mm_load_si128((__m128i*)(pLine0 + loopNo * 16)));
				vecImgVal[1] = _mm256_cvtepu8_epi16(_mm_load_si128((__m128i*)(pLine0 + loopNo * 16 + 1)));
				vecImgVal[2] = _mm256_cvtepu8_epi16(_mm_load_si128((__m128i*)(pLine0 + loopNo * 16 + 2)));
				vecImgVal[3] = _mm256_cvtepu8_epi16(_mm_load_si128((__m128i*)(pLine1 + loopNo * 16)));
				vecImgVal[4] = _mm256_cvtepu8_epi16(_mm_load_si128((__m128i*)(pLine1 + loopNo * 16 + 2)));
				vecImgVal[5] = _mm256_cvtepu8_epi16(_mm_load_si128((__m128i*)(pLine2 + loopNo * 16)));
				vecImgVal[6] = _mm256_cvtepu8_epi16(_mm_load_si128((__m128i*)(pLine2 + loopNo * 16 + 1)));
				vecImgVal[7] = _mm256_cvtepu8_epi16(_mm_load_si128((__m128i*)(pLine2 + loopNo * 16 + 2)));

				GX = _mm256_sub_epi16(_mm256_add_epi16(_mm256_add_epi16(vecImgVal[2], vecImgVal[7]), _mm256_slli_epi16(vecImgVal[4], 1)),
					_mm256_add_epi16(_mm256_add_epi16(vecImgVal[0], vecImgVal[5]), _mm256_slli_epi16(vecImgVal[3], 1)));
				GY = _mm256_sub_epi16(_mm256_add_epi16(_mm256_add_epi16(vecImgVal[5], vecImgVal[7]), _mm256_slli_epi16(vecImgVal[6], 1)),
					_mm256_add_epi16(_mm256_add_epi16(vecImgVal[0], vecImgVal[2]), _mm256_slli_epi16(vecImgVal[1], 1)));

				GX0 = _mm256_cvtepi32_ps(_mm256_cvtepi16_epi32(_mm256_castsi256_si128(GX)));
				GX1 = _mm256_cvtepi32_ps(_mm256_cvtepi16_epi32(_mm256_extracti128_si256(GX, 1)));
				GY0 = _mm256_cvtepi32_ps(_mm256_cvtepi16_epi32(_mm256_castsi256_si128(GY)));
				GY1 = _mm256_cvtepi32_ps(_mm256_cvtepi16_epi32(_mm256_extracti128_si256(GY, 1)));

				Mag0 = _mm256_sqrt_ps(_mm256_add_ps(_mm256_mul_ps(GX0, GX0), _mm256_mul_ps(GY0, GY0)));
				Mag1 = _mm256_sqrt_ps(_mm256_add_ps(_mm256_mul_ps(GX1, GX1), _mm256_mul_ps(GY1, GY1)));

				tmpMax0 = _mm256_max_ps(tmpMax0, Mag0);
				tmpMax1 = _mm256_max_ps(tmpMax1, Mag1);

				idx = row * inSrcImg.cols + 16 * loopNo;
				if (restNum != 0 && loopNo == loopNum - 1) {
					_mm256_store_ps(vecRestMag, Mag0);
					_mm256_store_ps(vecRestMag + 8, Mag1);
					for (int restNo = 0; restNo < restNum; ++restNo) {
						*(pMag + idx + restNo) = vecRestMag[restNo];
					}
				}
				else {
					_mm256_store_ps(pMag + idx, Mag0);
					_mm256_store_ps(pMag + idx + 8, Mag1);
				}
			}
		}

		float maxMag = 0;
		__m256 tmpMax = _mm256_max_ps(tmpMax0, tmpMax1);
		float vecTmpMax[8];
		_mm256_store_ps(vecTmpMax, tmpMax);
		for (int i = 0; i < 8; ++i) {
			if (vecTmpMax[i] > maxMag) maxMag = vecTmpMax[i];
		}
		float fMag = 1. / (maxMag + 1e-6);
		const int magLoopNum = outMagImg.total() / 8;
		__m256 mag;
		__m256 f = _mm256_set1_ps(fMag);
		for (int magLoopNo = 0; magLoopNo < magLoopNum; ++magLoopNo) {
			mag = _mm256_load_ps(pMag + magLoopNo * 8);
			mag = _mm256_mul_ps(mag, f);
			_mm256_store_ps(pMag + magLoopNo * 8, mag);
		}
		const int magRestNum = outMagImg.total() % 8;
		int beginIdx = magLoopNum * 8;
		for (int magRestNo = 0; magRestNo < magRestNum; ++magRestNo) {
			*(pMag + magLoopNum * 8 + magRestNo) = *(pMag + magLoopNum * 8 + magRestNo) * fMag;
		}

		if (outMaxMagPtr != nullptr) {
			*outMaxMagPtr = maxMag;
		}
#endif
	}

	 inline void getNormalizedGrayImage(const cv::Mat& inSrcImg, cv::Mat& outGrayImg) {
		cv::Mat tmpImg;
		inSrcImg.convertTo(tmpImg, CV_32FC1);
		outGrayImg = tmpImg / 255.;
	}

	 inline float calculateMagnitudeScore(const cv::Mat& inSrcImg, const cv::Mat& inTplX, const cv::Mat& inTplY, const float& inDX, const float& inDY) {
		const int ptNum = inTplX.cols;
#ifndef __AVX2__
		cv::Mat magImg = cv::Mat::zeros(inTplX.size(), inSrcImg.type());
		cv::remap(inSrcImg, magImg, inTplX + inDX, inTplY + inDY, cv::INTER_NEAREST);
		return cv::sum(magImg)[0] / float(ptNum);
#else
		__m256* pX = (__m256*)inTplX.ptr<float>();
		__m256* pY = (__m256*)inTplY.ptr<float>();
		__m256 DX = _mm256_set1_ps(inDX);
		__m256 DY = _mm256_set1_ps(inDY);
		__m256i minX = _mm256_set1_epi32(-1), maxX = _mm256_set1_epi32(inSrcImg.cols);
		__m256i minY = _mm256_set1_epi32(-1), maxY = _mm256_set1_epi32(inSrcImg.rows);
		__m256 zero_vector_ps = _mm256_setzero_ps();
		__m256i zero_vector_si256 = _mm256_setzero_si256();
		__m256 addRes = _mm256_setzero_ps();
		const int loopNum = ptNum / 8;
		for (int i = 0; i < loopNum; ++i) {
			__m256i roundX = _mm256_cvtps_epi32(_mm256_add_ps(pX[i], DX));
			__m256i roundY = _mm256_cvtps_epi32(_mm256_add_ps(pY[i], DY));
			__m256i mask = _mm256_and_si256(_mm256_and_si256(_mm256_cmpgt_epi32(roundX, minX), _mm256_cmpgt_epi32(maxX, roundX)), _mm256_and_si256(_mm256_cmpgt_epi32(roundY, minY), _mm256_cmpgt_epi32(maxY, roundY)));
			roundX = _mm256_blendv_epi8(zero_vector_si256, roundX, mask);
			roundY = _mm256_blendv_epi8(zero_vector_si256, roundY, mask);
			addRes = _mm256_add_ps(addRes, _mm256_blendv_ps(zero_vector_ps, _mm256_i32gather_ps(inSrcImg.ptr<float>(), _mm256_add_epi32(_mm256_mullo_epi32(roundY, maxX), roundX), 4), _mm256_castsi256_ps(mask)));
		}
		float tmpAddRes[8];
		_mm256_store_ps(tmpAddRes, addRes);
		float res = 0;
		for (int i = 0; i < 8; ++i) {
			res += tmpAddRes[i];
		}
		const int restNum = ptNum % 8;
		const int beginIdx = loopNum * 8;
		for (int i = 0; i < restNum; ++i) {
			const int x = cvRound(inTplX.ptr<float>()[beginIdx + i] + inDX);
			const int y = cvRound(inTplY.ptr<float>()[beginIdx + i] + inDY);
			if (x < 0 || x >= inSrcImg.cols || y < 0 || y >= inSrcImg.rows) {
				res += 0;
			}
			else {
				res += inSrcImg.ptr<float>(y)[x];
			}
		}
		return res / float(ptNum);
#endif
	}

	 inline float calculateGrayScore(const cv::Mat& inSrcImg, const cv::Mat& inTplX, const cv::Mat& inTplY, const float& inDX, const float& inDY) {
		const int ptNum = inTplX.cols;
#ifndef __AVX2__
		cv::Mat magImg = cv::Mat::zeros(inTplX.size(), inSrcImg.type());
		cv::remap(inSrcImg, magImg, inTplX + inDX, inTplY + inDY, cv::INTER_NEAREST);
		return cv::sum(magImg)[0] / float(ptNum);
#else
		__m256* pX = (__m256*)inTplX.ptr<float>();
		__m256* pY = (__m256*)inTplY.ptr<float>();
		__m256 DX = _mm256_set1_ps(inDX);
		__m256 DY = _mm256_set1_ps(inDY);
		__m256i minX = _mm256_set1_epi32(-1), maxX = _mm256_set1_epi32(inSrcImg.cols);
		__m256i minY = _mm256_set1_epi32(-1), maxY = _mm256_set1_epi32(inSrcImg.rows);
		__m256 zero_vector_ps = _mm256_setzero_ps();
		__m256i zero_vector_si256 = _mm256_setzero_si256();
		__m256 addRes = _mm256_setzero_ps();
		const int loopNum = ptNum / 8;
		for (int i = 0; i < loopNum; ++i) {
			__m256i roundX = _mm256_cvtps_epi32(_mm256_add_ps(pX[i], DX));
			__m256i roundY = _mm256_cvtps_epi32(_mm256_add_ps(pY[i], DY));
			__m256i mask = _mm256_and_si256(_mm256_and_si256(_mm256_cmpgt_epi32(roundX, minX), _mm256_cmpgt_epi32(maxX, roundX)), _mm256_and_si256(_mm256_cmpgt_epi32(roundY, minY), _mm256_cmpgt_epi32(maxY, roundY)));
			roundX = _mm256_blendv_epi8(zero_vector_si256, roundX, mask);
			roundY = _mm256_blendv_epi8(zero_vector_si256, roundY, mask);
			addRes = _mm256_add_ps(addRes, _mm256_blendv_ps(zero_vector_ps, _mm256_i32gather_ps(inSrcImg.ptr<float>(), _mm256_add_epi32(_mm256_mullo_epi32(roundY, maxX), roundX), 4), _mm256_castsi256_ps(mask)));
		}
		float tmpAddRes[8];
		_mm256_store_ps(tmpAddRes, addRes);
		float res = 0;
		for (int i = 0; i < 8; ++i) {
			res += tmpAddRes[i];
		}
		const int restNum = ptNum % 8;
		const int beginIdx = loopNum * 8;
		for (int i = 0; i < restNum; ++i) {
			const int x = cvRound(inTplX.ptr<float>()[beginIdx + i] + inDX);
			const int y = cvRound(inTplY.ptr<float>()[beginIdx + i] + inDY);
			if (x < 0 || x >= inSrcImg.cols || y < 0 || y >= inSrcImg.rows) {
				res += 0;
			}
			else {
				res += inSrcImg.ptr<float>(y)[x];
			}
		}
		return res / float(ptNum);
#endif
	}

	 inline float calculateGradientScore(const cv::Mat& inGradImgX, const cv::Mat& inGradImgY, const cv::Mat& inTplX, const cv::Mat& inTplY, const cv::Mat& inTplGX, const cv::Mat& inTplGY, const float& inDX, const float& inDY) {
		const int ptNum = inTplX.cols;
#ifndef __AVX2__
		cv::Mat imgGX = cv::Mat::zeros(inTplX.size(), inTplX.type());
		cv::Mat imgGY = cv::Mat::zeros(inTplX.size(), inTplX.type());
		cv::remap(inGradImgX, imgGX, inTplX + inDX, inTplY + inDY, cv::INTER_NEAREST);
		cv::remap(inGradImgY, imgGY, inTplX + inDX, inTplY + inDY, cv::INTER_NEAREST);
		return cv::sum(cv::abs(inTplGX.mul(imgGX) + inTplGY.mul(imgGY)))[0] / float(ptNum);
#else
		__m256* pTplX = (__m256*)inTplX.ptr<float>();
		__m256* pTplY = (__m256*)inTplY.ptr<float>();
		__m256* pTplGX = (__m256*)inTplGX.ptr<float>();
		__m256* pTplGY = (__m256*)inTplGY.ptr<float>();
		__m256 DX = _mm256_set1_ps(inDX);
		__m256 DY = _mm256_set1_ps(inDY);
		__m256i minX = _mm256_set1_epi32(-1), maxX = _mm256_set1_epi32(inGradImgX.cols);
		__m256i minY = _mm256_set1_epi32(-1), maxY = _mm256_set1_epi32(inGradImgX.rows);
		__m256 zero_vector_ps = _mm256_setzero_ps();
		__m256i zero_vector_si256 = _mm256_setzero_si256();
		__m256 absMask = _mm256_castsi256_ps(_mm256_set1_epi32(0x7FFFFFFF));
		__m256 addRes = _mm256_setzero_ps();
		const int loopNum = ptNum / 8;
		for (int i = 0; i < loopNum; ++i) {
			__m256i roundX = _mm256_cvtps_epi32(_mm256_add_ps(pTplX[i], DX));
			__m256i roundY = _mm256_cvtps_epi32(_mm256_add_ps(pTplY[i], DY));
			__m256i mask = _mm256_and_si256(_mm256_and_si256(_mm256_cmpgt_epi32(roundX, minX), _mm256_cmpgt_epi32(maxX, roundX)), _mm256_and_si256(_mm256_cmpgt_epi32(roundY, minY), _mm256_cmpgt_epi32(maxY, roundY)));
			roundX = _mm256_blendv_epi8(zero_vector_si256, roundX, mask);
			roundY = _mm256_blendv_epi8(zero_vector_si256, roundY, mask);
			__m256i memIdx = _mm256_add_epi32(_mm256_mullo_epi32(roundY, maxX), roundX);
			__m256 tmpValGX = _mm256_i32gather_ps(inGradImgX.ptr<float>(), memIdx, 4);
			__m256 tmpValGY = _mm256_i32gather_ps(inGradImgY.ptr<float>(), memIdx, 4);
			__m256 float_mask = _mm256_castsi256_ps(mask);
			tmpValGX = _mm256_blendv_ps(zero_vector_ps, tmpValGX, float_mask);
			tmpValGY = _mm256_blendv_ps(zero_vector_ps, tmpValGY, float_mask);
			addRes = _mm256_add_ps(addRes, _mm256_and_ps(_mm256_add_ps(_mm256_mul_ps(tmpValGX, pTplGX[i]), _mm256_mul_ps(tmpValGY, pTplGY[i])), absMask));
			//addRes = _mm256_add_ps(addRes, _mm256_add_ps(_mm256_mul_ps(tmpValGX, pTplGX[i]), _mm256_mul_ps(tmpValGY, pTplGY[i])));
		}
		float tmpAddRes[8];
		_mm256_store_ps(tmpAddRes, addRes);
		float res = 0;
		for (int i = 0; i < 8; ++i) {
			res += tmpAddRes[i];
		}
		const int restNum = ptNum % 8;
		const int beginIdx = loopNum * 8;
		for (int i = 0; i < restNum; ++i) {
			const int x = cvRound(inTplX.ptr<float>()[beginIdx + i] + inDX);
			const int y = cvRound(inTplY.ptr<float>()[beginIdx + i] + inDY);
			if (x < 0 || x >= inGradImgX.cols || y < 0 || y >= inGradImgX.rows) {
				res += 0;
			}
			else {
				res += abs((inGradImgX.ptr<float>(y)[x] * inTplGX.ptr<float>()[beginIdx + i]) + (inGradImgY.ptr<float>(y)[x] * inTplGY.ptr<float>()[beginIdx + i]));
			}
		}
		return res / ptNum;
#endif
	}

	 inline void calculateMagnitudeAndGradientScore(const cv::Mat& inMagImg, const cv::Mat& inGradImgX, const cv::Mat& inGradImgY, const cv::Mat& inTplX, const cv::Mat& inTplY, const cv::Mat& inTplGX, const cv::Mat& inTplGY, const float& inDX, const float& inDY, float& outMagScore, float& outGradScore) {
		const int ptNum = inTplX.cols;
#ifndef __AVX2__
		cv::Mat imgM = cv::Mat::zeros(inTplX.size(), inMagImg.type());
		cv::Mat imgGX = cv::Mat::zeros(inTplX.size(), inGradImgX.type());
		cv::Mat imgGY = cv::Mat::zeros(inTplX.size(), inGradImgY.type());
		cv::remap(inMagImg, imgM, inTplX + inDX, inTplY + inDY, cv::INTER_NEAREST);
		cv::remap(inGradImgX, imgGX, inTplX + inDX, inTplY + inDY, cv::INTER_NEAREST);
		cv::remap(inGradImgY, imgGY, inTplX + inDX, inTplY + inDY, cv::INTER_NEAREST);
		outMagScore = cv::sum(imgM)[0] / float(ptNum);
		outGradScore = cv::sum(cv::abs(inTplGX.mul(imgGX) + inTplGY.mul(imgGY)))[0] / float(ptNum);
#else
		__m256* pTplX = (__m256*)inTplX.ptr<float>();
		__m256* pTplY = (__m256*)inTplY.ptr<float>();
		__m256* pTplGX = (__m256*)inTplGX.ptr<float>();
		__m256* pTplGY = (__m256*)inTplGY.ptr<float>();
		__m256 DX = _mm256_set1_ps(inDX);
		__m256 DY = _mm256_set1_ps(inDY);
		__m256i minX = _mm256_set1_epi32(-1), maxX = _mm256_set1_epi32(inGradImgX.cols);
		__m256i minY = _mm256_set1_epi32(-1), maxY = _mm256_set1_epi32(inGradImgX.rows);
		__m256 zero_vector_ps = _mm256_setzero_ps();
		__m256i zero_vector_si256 = _mm256_setzero_si256();
		__m256 absMask = _mm256_castsi256_ps(_mm256_set1_epi32(0x7FFFFFFF));
		__m256 addResM = _mm256_setzero_ps();
		__m256 addResG = _mm256_setzero_ps();
		const int loopNum = ptNum / 8;
		for (int i = 0; i < loopNum; ++i) {
			__m256i roundX = _mm256_cvtps_epi32(_mm256_add_ps(pTplX[i], DX));
			__m256i roundY = _mm256_cvtps_epi32(_mm256_add_ps(pTplY[i], DY));
			__m256i mask = _mm256_and_si256(_mm256_and_si256(_mm256_cmpgt_epi32(roundX, minX), _mm256_cmpgt_epi32(maxX, roundX)), _mm256_and_si256(_mm256_cmpgt_epi32(roundY, minY), _mm256_cmpgt_epi32(maxY, roundY)));
			roundX = _mm256_blendv_epi8(zero_vector_si256, roundX, mask);
			roundY = _mm256_blendv_epi8(zero_vector_si256, roundY, mask);
			__m256i memIdx = _mm256_add_epi32(_mm256_mullo_epi32(roundY, maxX), roundX);
			__m256 tmpValM = _mm256_i32gather_ps(inMagImg.ptr<float>(), memIdx, 4);
			__m256 tmpValGX = _mm256_i32gather_ps(inGradImgX.ptr<float>(), memIdx, 4);
			__m256 tmpValGY = _mm256_i32gather_ps(inGradImgY.ptr<float>(), memIdx, 4);
			__m256 float_mask = _mm256_castsi256_ps(mask);
			tmpValM = _mm256_blendv_ps(zero_vector_ps, tmpValM, float_mask);
			tmpValGX = _mm256_blendv_ps(zero_vector_ps, tmpValGX, float_mask);
			tmpValGY = _mm256_blendv_ps(zero_vector_ps, tmpValGY, float_mask);
			addResM = _mm256_add_ps(addResM, tmpValM);
			addResG = _mm256_add_ps(addResG, _mm256_and_ps(_mm256_add_ps(_mm256_mul_ps(tmpValGX, pTplGX[i]), _mm256_mul_ps(tmpValGY, pTplGY[i])), absMask));
			//addResG = _mm256_add_ps(addResG, _mm256_add_ps(_mm256_mul_ps(tmpValGX, pTplGX[i]), _mm256_mul_ps(tmpValGY, pTplGY[i])));
	}
		float tmpAddResM[8];
		float tmpAddResG[8];
		_mm256_store_ps(tmpAddResM, addResM);
		_mm256_store_ps(tmpAddResG, addResG);
		float resM = 0;
		float resG = 0;
		for (int i = 0; i < 8; ++i) {
			resM += tmpAddResM[i];
			resG += tmpAddResG[i];
		}
		const int restNum = ptNum % 8;
		const int beginIdx = loopNum * 8;
		for (int i = 0; i < restNum; ++i) {
			const int x = cvRound(inTplX.ptr<float>()[beginIdx + i] + inDX);
			const int y = cvRound(inTplY.ptr<float>()[beginIdx + i] + inDY);
			if (x < 0 || x >= inGradImgX.cols || y < 0 || y >= inGradImgX.rows) {
				resM += 0;
				resG += 0;
			}
			else {
				resM += inMagImg.ptr<float>(y)[x];
				resG += abs((inGradImgX.ptr<float>(y)[x] * inTplGX.ptr<float>()[beginIdx + i]) + (inGradImgY.ptr<float>(y)[x] * inTplGY.ptr<float>()[beginIdx + i]));
			}
		}
		outMagScore = resM / ptNum;
		outGradScore = resG / ptNum;
#endif
	}

	 inline void calculateMatchScore(const cv::Mat& inMagImg, const cv::Mat& inGradImgX, const cv::Mat& inGradImgY, const cv::Mat& inTplX, const cv::Mat& inTplY, const cv::Mat& inTplGX, const cv::Mat& inTplGY, const float& inDX, const float& inDY, float& outMatchScore) {
		const int ptNum = inTplX.cols;
#ifndef __AVX2__
		cv::Mat imgM = cv::Mat::zeros(inTplX.size(), inMagImg.type());
		cv::Mat imgGX = cv::Mat::zeros(inTplX.size(), inGradImgX.type());
		cv::Mat imgGY = cv::Mat::zeros(inTplX.size(), inGradImgY.type());
		cv::remap(inMagImg, imgM, inTplX + inDX, inTplY + inDY, cv::INTER_NEAREST);
		cv::remap(inGradImgX, imgGX, inTplX + inDX, inTplY + inDY, cv::INTER_NEAREST);
		cv::remap(inGradImgY, imgGY, inTplX + inDX, inTplY + inDY, cv::INTER_NEAREST);
		outMatchScore = cv::sum(imgM.mul(cv::abs(inTplGX.mul(imgGX) + inTplGY.mul(imgGY))))[0] / float(ptNum);
#else
		__m256* pTplX = (__m256*)inTplX.ptr<float>();
		__m256* pTplY = (__m256*)inTplY.ptr<float>();
		__m256* pTplGX = (__m256*)inTplGX.ptr<float>();
		__m256* pTplGY = (__m256*)inTplGY.ptr<float>();
		__m256 DX = _mm256_set1_ps(inDX);
		__m256 DY = _mm256_set1_ps(inDY);
		__m256i minX = _mm256_set1_epi32(-1), maxX = _mm256_set1_epi32(inGradImgX.cols);
		__m256i minY = _mm256_set1_epi32(-1), maxY = _mm256_set1_epi32(inGradImgX.rows);
		__m256 zero_vector_ps = _mm256_setzero_ps();
		__m256i zero_vector_si256 = _mm256_setzero_si256();
		__m256 absMask = _mm256_castsi256_ps(_mm256_set1_epi32(0x7FFFFFFF));
		__m256 addRes = _mm256_setzero_ps();
		const int loopNum = ptNum / 8;
		for (int i = 0; i < loopNum; ++i) {
			__m256i roundX = _mm256_cvtps_epi32(_mm256_add_ps(pTplX[i], DX));
			__m256i roundY = _mm256_cvtps_epi32(_mm256_add_ps(pTplY[i], DY));
			__m256i mask = _mm256_and_si256(_mm256_and_si256(_mm256_cmpgt_epi32(roundX, minX), _mm256_cmpgt_epi32(maxX, roundX)), _mm256_and_si256(_mm256_cmpgt_epi32(roundY, minY), _mm256_cmpgt_epi32(maxY, roundY)));
			roundX = _mm256_blendv_epi8(zero_vector_si256, roundX, mask);
			roundY = _mm256_blendv_epi8(zero_vector_si256, roundY, mask);
			__m256i memIdx = _mm256_add_epi32(_mm256_mullo_epi32(roundY, maxX), roundX);
			__m256 tmpValM = _mm256_i32gather_ps(inMagImg.ptr<float>(), memIdx, 4);
			__m256 tmpValGX = _mm256_i32gather_ps(inGradImgX.ptr<float>(), memIdx, 4);
			__m256 tmpValGY = _mm256_i32gather_ps(inGradImgY.ptr<float>(), memIdx, 4);
			__m256 float_mask = _mm256_castsi256_ps(mask);
			tmpValM = _mm256_blendv_ps(zero_vector_ps, tmpValM, float_mask);
			tmpValGX = _mm256_blendv_ps(zero_vector_ps, tmpValGX, float_mask);
			tmpValGY = _mm256_blendv_ps(zero_vector_ps, tmpValGY, float_mask);
			__m256 tmpValG = _mm256_and_ps(_mm256_add_ps(_mm256_mul_ps(tmpValGX, pTplGX[i]), _mm256_mul_ps(tmpValGY, pTplGY[i])), absMask);
			addRes = _mm256_add_ps(addRes, _mm256_mul_ps(tmpValM, tmpValG));
		}
		float tmpAddRes[8];
		_mm256_store_ps(tmpAddRes, addRes);
		float res = 0;
		for (int i = 0; i < 8; ++i) {
			res += tmpAddRes[i];
		}
		const int restNum = ptNum % 8;
		const int beginIdx = loopNum * 8;
		for (int i = 0; i < restNum; ++i) {
			const int x = cvRound(inTplX.ptr<float>()[beginIdx + i] + inDX);
			const int y = cvRound(inTplY.ptr<float>()[beginIdx + i] + inDY);
			if (x < 0 || x >= inGradImgX.cols || y < 0 || y >= inGradImgX.rows) {
				res += 0;
			}
			else {
				res += inMagImg.ptr<float>(y)[x] * abs((inGradImgX.ptr<float>(y)[x] * inTplGX.ptr<float>()[beginIdx + i]) + (inGradImgY.ptr<float>(y)[x] * inTplGY.ptr<float>()[beginIdx + i]));
			}
		}
		outMatchScore = res / ptNum;
#endif
	}

	 inline void getMagnitudeImage(const cv::Mat& inSrcImg, cv::Mat& outMagImg, const int& inSobelSize = 3) {
#ifndef __AVX2__
		cv::Mat gradRoiImgX = cv::Mat(inSrcImg.size(), CV_32FC1);
		cv::Mat gradRoiImgY = cv::Mat(inSrcImg.size(), CV_32FC1);
		cv::Sobel(inSrcImg, gradRoiImgX, CV_32F, 1, 0, inSobelSize);
		cv::Sobel(inSrcImg, gradRoiImgY, CV_32F, 0, 1, inSobelSize);
		cv::Mat magnitudeImg;
		cv::magnitude(gradRoiImgX, gradRoiImgY, outMagImg);
#else
		outMagImg = cv::Mat(inSrcImg.size(), CV_32FC1);
		float* pMag = (float*)outMagImg.data;

		const int loopNum = ceilf((inSrcImg.cols) / (float)16);
		const int restNum = inSrcImg.cols % 16;
		const int blockSize = loopNum * 16 + 2;
		cv::Mat buffer = cv::Mat(cv::Size(blockSize * 3, 1), CV_8UC1);
		uchar* pBuffer = (uchar*)buffer.data;
		uchar* pSrcImg = inSrcImg.data;

		if (pBuffer == NULL) {
			return;
		}

		uchar* pLine0 = pBuffer;
		uchar* pLine1 = pBuffer + blockSize;
		uchar* pLine2 = pBuffer + blockSize * 2;

		memset(pLine0, *(pSrcImg), 1);
		memcpy(pLine0 + 1, pSrcImg, inSrcImg.cols);
		memset(pLine0 + 1 + inSrcImg.cols, *(pSrcImg + inSrcImg.cols - 1), blockSize - inSrcImg.cols - 1);
		memcpy(pLine1, pLine0, blockSize);

		__m256i vecImgVal[8];
		__m256i GX, GY;
		float vecRestGX[16];
		float vecRestGY[16];
		float vecRestMag[16];
		__m256 GX0, GX1, GY0, GY1;
		__m256 Mag0, Mag1;
		__m256 fMag0, fMag1;
		__m256 tmpMax0 = _mm256_setzero_ps(), tmpMax1 = _mm256_setzero_ps();
		__m256 delta = _mm256_set1_ps(1e-6);
		int idx;
		for (int row = 0; row < inSrcImg.rows; ++row) {
			pLine0 = pBuffer + (row % 3) * blockSize;
			pLine1 = pBuffer + ((row + 1) % 3) * blockSize;
			pLine2 = pBuffer + ((row + 2) % 3) * blockSize;
			if (row != inSrcImg.rows - 1) {
				memset(pLine2, *(pSrcImg + (row + 1) * inSrcImg.cols), 1);
				memcpy(pLine2 + 1, pSrcImg + (row + 1) * inSrcImg.cols, inSrcImg.cols);
				memset(pLine2 + 1 + inSrcImg.cols, *(pSrcImg + (row + 2) * inSrcImg.cols - 1), blockSize - inSrcImg.cols - 1);
			}
			else {
				memcpy(pLine2, pLine1, blockSize);
			}

			for (int loopNo = 0; loopNo < loopNum; ++loopNo) {
				vecImgVal[0] = _mm256_cvtepu8_epi16(_mm_load_si128((__m128i*)(pLine0 + loopNo * 16)));
				vecImgVal[1] = _mm256_cvtepu8_epi16(_mm_load_si128((__m128i*)(pLine0 + loopNo * 16 + 1)));
				vecImgVal[2] = _mm256_cvtepu8_epi16(_mm_load_si128((__m128i*)(pLine0 + loopNo * 16 + 2)));
				vecImgVal[3] = _mm256_cvtepu8_epi16(_mm_load_si128((__m128i*)(pLine1 + loopNo * 16)));
				vecImgVal[4] = _mm256_cvtepu8_epi16(_mm_load_si128((__m128i*)(pLine1 + loopNo * 16 + 2)));
				vecImgVal[5] = _mm256_cvtepu8_epi16(_mm_load_si128((__m128i*)(pLine2 + loopNo * 16)));
				vecImgVal[6] = _mm256_cvtepu8_epi16(_mm_load_si128((__m128i*)(pLine2 + loopNo * 16 + 1)));
				vecImgVal[7] = _mm256_cvtepu8_epi16(_mm_load_si128((__m128i*)(pLine2 + loopNo * 16 + 2)));

				GX = _mm256_sub_epi16(_mm256_add_epi16(_mm256_add_epi16(vecImgVal[2], vecImgVal[7]), _mm256_slli_epi16(vecImgVal[4], 1)),
					_mm256_add_epi16(_mm256_add_epi16(vecImgVal[0], vecImgVal[5]), _mm256_slli_epi16(vecImgVal[3], 1)));
				GY = _mm256_sub_epi16(_mm256_add_epi16(_mm256_add_epi16(vecImgVal[5], vecImgVal[7]), _mm256_slli_epi16(vecImgVal[6], 1)),
					_mm256_add_epi16(_mm256_add_epi16(vecImgVal[0], vecImgVal[2]), _mm256_slli_epi16(vecImgVal[1], 1)));

				GX0 = _mm256_cvtepi32_ps(_mm256_cvtepi16_epi32(_mm256_castsi256_si128(GX)));
				GX1 = _mm256_cvtepi32_ps(_mm256_cvtepi16_epi32(_mm256_extracti128_si256(GX, 1)));
				GY0 = _mm256_cvtepi32_ps(_mm256_cvtepi16_epi32(_mm256_castsi256_si128(GY)));
				GY1 = _mm256_cvtepi32_ps(_mm256_cvtepi16_epi32(_mm256_extracti128_si256(GY, 1)));

				Mag0 = _mm256_sqrt_ps(_mm256_add_ps(_mm256_mul_ps(GX0, GX0), _mm256_mul_ps(GY0, GY0)));
				Mag1 = _mm256_sqrt_ps(_mm256_add_ps(_mm256_mul_ps(GX1, GX1), _mm256_mul_ps(GY1, GY1)));

				idx = row * inSrcImg.cols + 16 * loopNo;
				if (restNum != 0 && loopNo == loopNum - 1) {
					_mm256_store_ps(vecRestMag, Mag0);
					_mm256_store_ps(vecRestMag + 8, Mag1);
					for (int restNo = 0; restNo < restNum; ++restNo) {
						*(pMag + idx + restNo) = vecRestMag[restNo];
					}
				}
				else {
					_mm256_store_ps(pMag + idx, Mag0);
					_mm256_store_ps(pMag + idx + 8, Mag1);
				}
			}
		}
#endif
	}

	 inline void getMaxBlurImage(const cv::Mat& inSrcImg, cv::Mat& outBlurImg) {
#ifndef __AVX2__
		cv::Mat tmpImg = inSrcImg.clone();
		cv::copyMakeBorder(tmpImg, tmpImg, 1, 1, 1, 1, cv::BORDER_CONSTANT, cv::Scalar(0));

		cv::Mat resImg = cv::Mat::zeros(inSrcImg.size(), CV_32FC1);
		for (int row = 0; row < resImg.rows; ++row) {
			float* pRow = resImg.ptr<float>(row);
			float* pRow0 = tmpImg.ptr<float>(row);
			float* pRow1 = tmpImg.ptr<float>(row + 1);
			float* pRow2 = tmpImg.ptr<float>(row + 2);
			for (int col = 0; col < resImg.cols; ++col) {
				pRow[col] = std::max({ pRow0[col], pRow0[col + 1], pRow0[col + 2], pRow1[col], pRow1[col + 1], pRow1[col + 2], pRow2[col], pRow2[col + 1], pRow2[col + 2] });
			}
		}

		outBlurImg = resImg;
#else
		cv::Mat tmpImg = inSrcImg.clone();
		outBlurImg = cv::Mat(inSrcImg.size(), CV_32FC1);
		float* pBlur = (float*)outBlurImg.data;

		const int loopNum = ceilf(inSrcImg.cols / float(8));
		const int restNum = inSrcImg.cols % 8;
		const int blockSize = loopNum * 8 + 2;
		cv::Mat buffer = cv::Mat(cv::Size(blockSize * 3, 1), CV_32FC1);
		float* pBuffer = (float*)buffer.data;
		float* pSrcImg = (float*)tmpImg.data;

		if (pBuffer == NULL) {
			return;
		}

		float* pLine0 = pBuffer;
		float* pLine1 = pBuffer + blockSize;
		float* pLine2 = pBuffer + blockSize * 2;

		memcpy(pLine0, pSrcImg, (1) * 4);
		memcpy(pLine0 + 1, pSrcImg, (inSrcImg.cols) * 4);
		for (int i = 0; i < blockSize - inSrcImg.cols - 1; ++i) {
			*(pLine0 + 1 + inSrcImg.cols + i) = *(pSrcImg + inSrcImg.cols - 1);
		}
		memcpy(pLine1, pLine0, (blockSize) * 4);

		__m256 vecImgVal[9];
		__m256 tmpMaxVal;
		float vecRestMax[8];
		int idx;
		for (int row = 0; row < inSrcImg.rows; ++row) {
			pLine0 = pBuffer + (row % 3) * blockSize;
			pLine1 = pBuffer + ((row + 1) % 3) * blockSize;
			pLine2 = pBuffer + ((row + 2) % 3) * blockSize;
			if (row != inSrcImg.rows - 1) {
				memcpy(pLine2, pSrcImg + (row + 1) * inSrcImg.cols, (1) * 4);
				memcpy(pLine2 + 1, pSrcImg + (row + 1) * inSrcImg.cols, (inSrcImg.cols) * 4);
				for (int i = 0; i < blockSize - inSrcImg.cols - 1; ++i) {
					*(pLine2 + 1 + inSrcImg.cols + i) = *(pSrcImg + (row + 2) * inSrcImg.cols - 1);
				}
			}
			else {
				memcpy(pLine2, pLine1, (blockSize) * 4);
			}

			for (int loopNo = 0; loopNo < loopNum; ++loopNo) {
				vecImgVal[0] = _mm256_load_ps(pLine0 + loopNo * 8);
				vecImgVal[1] = _mm256_load_ps(pLine0 + loopNo * 8 + 1);
				vecImgVal[2] = _mm256_load_ps(pLine0 + loopNo * 8 + 2);
				vecImgVal[3] = _mm256_load_ps(pLine1 + loopNo * 8);
				vecImgVal[4] = _mm256_load_ps(pLine1 + loopNo * 8 + 1);
				vecImgVal[5] = _mm256_load_ps(pLine1 + loopNo * 8 + 2);
				vecImgVal[6] = _mm256_load_ps(pLine2 + loopNo * 8);
				vecImgVal[7] = _mm256_load_ps(pLine2 + loopNo * 8 + 1);
				vecImgVal[8] = _mm256_load_ps(pLine2 + loopNo * 8 + 2);

				tmpMaxVal = _mm256_setzero_ps();
				for (int i = 0; i < 9; ++i) {
					tmpMaxVal = _mm256_max_ps(tmpMaxVal, vecImgVal[i]);
				}

				idx = row * inSrcImg.cols + 8 * loopNo;
				if (restNum != 0 && loopNo == loopNum - 1) {
					_mm256_store_ps(vecRestMax, tmpMaxVal);
					for (int restNo = 0; restNo < restNum; ++restNo) {
						*(pBlur + idx + restNo) = vecRestMax[restNo];
					}
				}
				else {
					_mm256_store_ps(pBlur + idx, tmpMaxVal);
				}
			}
	}
#endif
}

	 inline void getMinBlurImage(const cv::Mat& inSrcImg, cv::Mat& outBlurImg) {
#ifndef __AVX2__
		cv::Mat tmpImg = inSrcImg.clone();
		cv::copyMakeBorder(tmpImg, tmpImg, 1, 1, 1, 1, cv::BORDER_CONSTANT, cv::Scalar(0));

		cv::Mat resImg = std::numeric_limits<float>::max() * cv::Mat::ones(inSrcImg.size(), CV_32FC1);
		for (int row = 0; row < resImg.rows; ++row) {
			float* pRow = resImg.ptr<float>(row);
			float* pRow0 = tmpImg.ptr<float>(row);
			float* pRow1 = tmpImg.ptr<float>(row + 1);
			float* pRow2 = tmpImg.ptr<float>(row + 2);
			for (int col = 0; col < resImg.cols; ++col) {
				pRow[col] = std::min({ pRow0[col], pRow0[col + 1], pRow0[col + 2], pRow1[col], pRow1[col + 1], pRow1[col + 2], pRow2[col], pRow2[col + 1], pRow2[col + 2] });
			}
		}

		outBlurImg = resImg;
#else
		cv::Mat tmpImg = inSrcImg.clone();
		outBlurImg = cv::Mat(inSrcImg.size(), CV_32FC1);
		float* pBlur = (float*)outBlurImg.data;

		const int loopNum = ceilf(inSrcImg.cols / float(8));
		const int restNum = inSrcImg.cols % 8;
		const int blockSize = loopNum * 8 + 2;
		cv::Mat buffer = cv::Mat(cv::Size(blockSize * 3, 1), CV_32FC1);
		float* pBuffer = (float*)buffer.data;
		float* pSrcImg = (float*)tmpImg.data;

		if (pBuffer == NULL) {
			return;
		}

		float* pLine0 = pBuffer;
		float* pLine1 = pBuffer + blockSize;
		float* pLine2 = pBuffer + blockSize * 2;

		memcpy(pLine0, pSrcImg, (1) * 4);
		memcpy(pLine0 + 1, pSrcImg, (inSrcImg.cols) * 4);
		for (int i = 0; i < blockSize - inSrcImg.cols - 1; ++i) {
			*(pLine0 + 1 + inSrcImg.cols + i) = *(pSrcImg + inSrcImg.cols - 1);
		}
		memcpy(pLine1, pLine0, (blockSize) * 4);

		__m256 vecImgVal[9];
		__m256 tmpMaxVal;
		float vecRestMax[8];
		int idx;
		for (int row = 0; row < inSrcImg.rows; ++row) {
			pLine0 = pBuffer + (row % 3) * blockSize;
			pLine1 = pBuffer + ((row + 1) % 3) * blockSize;
			pLine2 = pBuffer + ((row + 2) % 3) * blockSize;
			if (row != inSrcImg.rows - 1) {
				memcpy(pLine2, pSrcImg + (row + 1) * inSrcImg.cols, (1) * 4);
				memcpy(pLine2 + 1, pSrcImg + (row + 1) * inSrcImg.cols, (inSrcImg.cols) * 4);
				for (int i = 0; i < blockSize - inSrcImg.cols - 1; ++i) {
					*(pLine2 + 1 + inSrcImg.cols) = *(pSrcImg + (row + 2) * inSrcImg.cols - 1);
				}
			}
			else {
				memcpy(pLine2, pLine1, (blockSize) * 4);
			}

			for (int loopNo = 0; loopNo < loopNum; ++loopNo) {
				vecImgVal[0] = _mm256_load_ps(pLine0 + loopNo * 8);
				vecImgVal[1] = _mm256_load_ps(pLine0 + loopNo * 8 + 1);
				vecImgVal[2] = _mm256_load_ps(pLine0 + loopNo * 8 + 2);
				vecImgVal[3] = _mm256_load_ps(pLine1 + loopNo * 8);
				vecImgVal[4] = _mm256_load_ps(pLine1 + loopNo * 8 + 1);
				vecImgVal[5] = _mm256_load_ps(pLine1 + loopNo * 8 + 2);
				vecImgVal[6] = _mm256_load_ps(pLine2 + loopNo * 8);
				vecImgVal[7] = _mm256_load_ps(pLine2 + loopNo * 8 + 1);
				vecImgVal[8] = _mm256_load_ps(pLine2 + loopNo * 8 + 2);
				tmpMaxVal = _mm256_set_ps((std::numeric_limits<float>::max)(), (std::numeric_limits<float>::max)(), (std::numeric_limits<float>::max)(), (std::numeric_limits<float>::max)(), (std::numeric_limits<float>::max)(), (std::numeric_limits<float>::max)(), (std::numeric_limits<float>::max)(), (std::numeric_limits<float>::max)());
				for (int i = 0; i < 9; ++i) {
					tmpMaxVal = _mm256_min_ps(tmpMaxVal, vecImgVal[i]);
				}

				idx = row * inSrcImg.cols + 8 * loopNo;
				if (restNum != 0 && loopNo == loopNum - 1) {
					_mm256_store_ps(vecRestMax, tmpMaxVal);
					for (int restNo = 0; restNo < restNum; ++restNo) {
						*(pBlur + idx + restNo) = vecRestMax[restNo];
					}
				}
				else {
					_mm256_store_ps(pBlur + idx, tmpMaxVal);
				}
			}
		}
#endif
	}

	 inline void getInterpolationMagnitudeMap(const cv::Mat& inMagImg, const cv::Mat& inTplX, const cv::Mat& inTplY, cv::Mat& inTplGX, const cv::Mat& inTplGY, const int& inIpW, cv::Mat& outMagRes) {
		const int ptNum = inTplX.cols;
		const int ipNum = 2 * inIpW + 1;
#ifndef __AVX2__

#else
		float* pMag = (float*)inMagImg.data;
		outMagRes = cv::Mat(cv::Size(ptNum, ipNum), CV_32FC1);
		float* pRes = (float*)outMagRes.data;
		__m256* pTplX = (__m256*)inTplX.ptr<float>();
		__m256* pTplY = (__m256*)inTplY.ptr<float>();
		__m256* pTplGX = (__m256*)inTplGX.ptr<float>();
		__m256* pTplGY = (__m256*)inTplGY.ptr<float>();
		__m256i minX = _mm256_set1_epi32(-1), maxX = _mm256_set1_epi32(inMagImg.cols);
		__m256i minY = _mm256_set1_epi32(-1), maxY = _mm256_set1_epi32(inMagImg.rows);
		__m256i zero_vector_si256 = _mm256_setzero_si256();
		__m256 zero_vector_ps = _mm256_setzero_ps();
		const int loopNum = ptNum / 8;
		for (int i = 0; i < loopNum; ++i) {
			for (int j = 0; j < ipNum; ++j) {
				__m256 delta = _mm256_set1_ps(-inIpW + j);
				__m256i roundX = _mm256_cvtps_epi32(_mm256_add_ps(pTplX[i], _mm256_mul_ps(delta, pTplGX[i])));
				__m256i roundY = _mm256_cvtps_epi32(_mm256_add_ps(pTplY[i], _mm256_mul_ps(delta, pTplGY[i])));
				__m256i mask = _mm256_and_si256(_mm256_and_si256(_mm256_cmpgt_epi32(roundX, minX), _mm256_cmpgt_epi32(maxX, roundX)), _mm256_and_si256(_mm256_cmpgt_epi32(roundY, minY), _mm256_cmpgt_epi32(maxY, roundY)));
				roundX = _mm256_blendv_epi8(zero_vector_si256, roundX, mask);
				roundY = _mm256_blendv_epi8(zero_vector_si256, roundY, mask);
				__m256i memIdx = _mm256_add_epi32(_mm256_mullo_epi32(roundY, maxX), roundX);
				__m256 tmpValM = _mm256_i32gather_ps(inMagImg.ptr<float>(), memIdx, 4);
				__m256 float_mask = _mm256_castsi256_ps(mask);
				tmpValM = _mm256_blendv_ps(zero_vector_ps, tmpValM, float_mask);
				_mm256_store_ps(pRes + j * ptNum + i * 8, tmpValM);
			}
		}

		const int restNum = ptNum % 8;
		for (int i = 0; i < restNum; ++i) {
			const int ptNo = ptNum - restNum + i;
			for (int j = 0; j < ipNum; ++j) {
				float delta = -inIpW + j;
				int x = round(inTplX.ptr<float>()[ptNo] + delta * inTplGX.ptr<float>()[ptNo]);
				int y = round(inTplY.ptr<float>()[ptNo] + delta * inTplGY.ptr<float>()[ptNo]);
				if (x > -1 && x < inMagImg.cols && y > -1 && y < inMagImg.rows) {
					outMagRes.ptr<float>(j)[ptNo] = inMagImg.ptr<float>(y)[x];
				}
				else {
					outMagRes.ptr<float>(j)[ptNo] = 0;
				}
			}
		}
#endif
	}
}

#endif
