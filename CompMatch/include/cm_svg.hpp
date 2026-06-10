#pragma once
#ifndef CM_SVG_HPP
#define CM_SVG_HPP
#include <string>
#include <fstream>
#include <opencv2/opencv.hpp>
#include "cm_error_code.hpp"


// Writes lightweight SVG debug overlays with optional image backgrounds.
class SVGTool {
public:
	// Use images as the background.
	SVGTool(const std::string& inImgPath, const cv::Mat& inBgImg)
		: ofs(inImgPath, std::ios::out | std::ios::binary)
	{
		if (!ofs.is_open()) {
			throw std::runtime_error("Failed to open file: " + inImgPath);
		}

		const int width = inBgImg.cols;
		const int height = inBgImg.rows;

		cv::Mat norImg;
		if (inBgImg.type() == CV_32F) {
			cv::normalize(inBgImg, norImg, 0, 255, cv::NORM_MINMAX, CV_8UC1);
		}
		else {
			norImg = inBgImg;
		}

		std::vector<uchar> buf;
		cv::imencode(".png", norImg, buf);

		const std::string base64_data = base64_encode(buf.data(), buf.size());

		ofs << R"(<?xml version="1.0" encoding="UTF-8"?>)" << "\n";
		ofs << "<svg xmlns=\"http://www.w3.org/2000/svg\" "
			"xmlns:xlink=\"http://www.w3.org/1999/xlink\" "
			"width=\"" << width << "\" height=\"" << height << "\" "
			"viewBox=\"0 0 " << width << " " << height << "\">\n";
		ofs << "<image x=\"0\" y=\"0\" width=\"" << width << "\" height=\"" << height
			<< "\" xlink:href=\"data:image/png;base64," << base64_data << "\" />\n";
	}

	// Use a blank background
	SVGTool(const std::string& inImgPath, int inWidth, int inHeight)
		: ofs(inImgPath, std::ios::out | std::ios::binary)
	{
		if (!ofs.is_open()) {
			throw std::runtime_error("Failed to open file: " + inImgPath);
		}
		ofs << R"(<?xml version="1.0" encoding="UTF-8"?>)" << "\n";
		ofs << "<svg xmlns=\"http://www.w3.org/2000/svg\" "
			"xmlns:xlink=\"http://www.w3.org/1999/xlink\" "
			"width=\"" << inWidth << "\" height=\"" << inHeight << "\" "
			"viewBox=\"0 0 " << inWidth << " " << inHeight << "\">\n";
	}

	// Draw a filled or stroked circle.
	void drawCircle(const cv::Point2f& inPos, const float& inR, const std::string& inColor, const float& inWidth = -1) {
		if (inWidth <= 0) {
			ofs << "<circle cx=\"" << inPos.x
				<< "\" cy=\"" << inPos.y
				<< "\" r=\"" << inR
				<< "\" fill=\"" << inColor
				<< "\" />\n";
		}
		else {
			ofs << "<circle cx=\"" << inPos.x
				<< "\" cy=\"" << inPos.y
				<< "\" r=\"" << inR
				<< "\" fill=\"none\""
				<< " stroke=\"" << inColor
				<< "\" stroke-width=\"" << inWidth
				<< "\" />\n";
		}
	}

	// Draw a straight line segment.
	void drawLine(const cv::Point2f& inPt1, const cv::Point2f& inPt2, const float& inTk, const std::string& inColor) {
		ofs << "<line x1=\"" << inPt1.x
			<< "\" y1=\"" << inPt1.y
			<< "\" x2=\"" << inPt2.x
			<< "\" y2=\"" << inPt2.y
			<< "\" stroke=\"" << inColor
			<< "\" stroke-width=\"" << inTk
			<< "\" />\n";
	}

	// Draw a rotated rectangle.
	void drawRect(const cv::Point2f& inCtr, const cv::Size2f inSize, const float& inAngle, const float& inTk, const std::string& inColor, const std::string& inFill = "none") {
		ofs << "<rect x=\"" << inCtr.x - inSize.width * 0.5
			<< "\" y=\"" << inCtr.y - inSize.height * 0.5
			<< "\" width=\"" << inSize.width
			<< "\" height=\"" << inSize.height
			<< "\" fill=\"" << inFill
			<< "\" stroke=\"" << inColor
			<< "\" stroke-width=\"" << inTk
			<< "\" transform=\"rotate(" << inAngle << "," << inCtr.x << "," << inCtr.y
			<< ")\" />\n";
	}

	// Draw text at an image-space position.
	void drawText(const cv::Point2f& inPos, const std::string& inColor, const std::string& inSize, const std::string& inText, const std::string& inFamily = "Arial", const std::string& inFill = "none") {
		ofs << "<text x=\"" << inPos.x
			<< "\" y=\"" << inPos.y
			<< "\" fill=\"" << inColor
			<< "\" font-family=\"" << inFamily
			<< "\" font-size=\"" << inSize
			<< "\" >" << inText
			<< "</text>\n";
	}

	// Finish and close the SVG document.
	void close() {
		if (ofs.is_open()) {
			ofs << "</svg>\n";
			ofs.close();
		}
	}

	~SVGTool() {
		if (ofs.is_open()) {
			ofs << "</svg>\n";
			ofs.close();
		}
	}

private:
	// Encode PNG bytes for embedding in the SVG.
	std::string base64_encode(const unsigned char* bytes_to_encode, size_t in_len) {
		static const std::string base64_chars =
			"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
			"abcdefghijklmnopqrstuvwxyz"
			"0123456789+/";
		std::string ret;
		int i = 0;
		unsigned char char_array_3[3];
		unsigned char char_array_4[4];

		while (in_len--) {
			char_array_3[i++] = *(bytes_to_encode++);
			if (i == 3) {
				char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
				char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
				char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
				char_array_4[3] = char_array_3[2] & 0x3f;

				for (i = 0; i < 4; i++)
					ret += base64_chars[char_array_4[i]];
				i = 0;
			}
		}

		if (i) {
			for (int j = i; j < 3; j++)
				char_array_3[j] = '\0';

			char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
			char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
			char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
			char_array_4[3] = char_array_3[2] & 0x3f;

			for (int j = 0; j < i + 1; j++)
				ret += base64_chars[char_array_4[j]];

			while (i++ < 3)
				ret += '=';
		}

		return ret;
	}

private:
	std::ofstream ofs;
};

// Represents a simple rectangular SVG primitive with rotation.
struct SVGItem {
	SVGItem() : cx(0.), cy(0.), w(0.), h(0.), r(0.) {};

	SVGItem(const float& inCx, const float& inCy, const float& inW, const float& inH, const float& inR) :
		cx(inCx), cy(inCy), w(inW), h(inH), r(inR) {};

	float cx, cy, w, h, r;

	SVGItem clone() const {
		return SVGItem(cx, cy, w, h, r);
	}
};

// Translate one SVG item by an offset.
inline AnswerType GetTranslatedItem(const SVGItem& inSrcItem, SVGItem& outDstItem, const cv::Point2f& inOffset) {
	AnswerType answer = IMG_SUCCESS_ANS().SetErrCode();

	SVGItem tmpItem = inSrcItem.clone();
	tmpItem.cx += inOffset.x;
	tmpItem.cy += inOffset.y;
	outDstItem = tmpItem;

	return answer;
}

// Translate a list of SVG items by an offset.
inline AnswerType GetTranslatedItems(const std::vector<SVGItem>& inVecSrcItem, std::vector<SVGItem>& outVecDstItem, const cv::Point2f& inOffset) {
	AnswerType answer = IMG_SUCCESS_ANS().SetErrCode();

	std::vector<SVGItem> vecDstItem;
	for (int i = 0; i < inVecSrcItem.size(); ++i) {
		SVGItem dstItem;
		GetTranslatedItem(inVecSrcItem[i], dstItem, inOffset);
		vecDstItem.push_back(dstItem);
	}
	outVecDstItem = vecDstItem;

	return answer;
}

// Rotate one SVG item around the origin.
inline AnswerType GetRotatedItem(const SVGItem& inSrcItem, SVGItem& outDstItem, const float& inAng) {
	AnswerType answer = IMG_SUCCESS_ANS().SetErrCode();

	SVGItem tmpItem = inSrcItem.clone();
	const float RT = CV_PI / 180.;
	const float r = inAng * RT;
	const float sin = std::sin(r);
	const float cos = std::cos(r);
	tmpItem.cx = cos * inSrcItem.cx - sin * inSrcItem.cy;
	tmpItem.cy = sin * inSrcItem.cx + cos * inSrcItem.cy;
	tmpItem.r += inAng;
	outDstItem = tmpItem;

	return answer;
}

// Rotate a list of SVG items around the origin.
inline AnswerType GetRotatedItems(const std::vector<SVGItem>& inVecSrcItem, std::vector<SVGItem>& outVecDstItem, const float& inAng) {
	AnswerType answer = IMG_SUCCESS_ANS().SetErrCode();

	std::vector<SVGItem> vecDstItem;
	for (int i = 0; i < inVecSrcItem.size(); ++i) {
		SVGItem dstItem;
		GetRotatedItem(inVecSrcItem[i], dstItem, inAng);
		vecDstItem.push_back(dstItem);
	}
	outVecDstItem = vecDstItem;

	return answer;
}

// Merge grouped SVG items into one flat list.
inline AnswerType GetMergedItems(const std::vector<std::vector<SVGItem>>& inVecSrcItem, std::vector<SVGItem>& outVecDstItem) {
	AnswerType answer = IMG_SUCCESS_ANS().SetErrCode();

	std::vector<SVGItem> vecDstItem;
	for (int i = 0; i < inVecSrcItem.size(); ++i) {
		vecDstItem.insert(vecDstItem.end(), inVecSrcItem[i].begin(), inVecSrcItem[i].end());
	}
	outVecDstItem = vecDstItem;

	return answer;
}

#endif
