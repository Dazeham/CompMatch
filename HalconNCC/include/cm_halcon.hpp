#pragma once
#ifndef CM_HALCON_HPP
#define CM_HALCON_HPP
#include <vector>
#include "HalconCpp.h"


namespace {
    template<typename> inline constexpr bool always_false = false;
}

namespace cm {
    
template<typename T>
    inline std::vector<T> tupleToVector(const HalconCpp::HTuple& inTuple) {
        std::vector<T> result;
        result.reserve(inTuple.Length());
        for (Hlong i = 0; i < inTuple.Length(); ++i) {
            if constexpr (std::is_same_v<T, double>) {
                result.push_back(inTuple[i].D());
            }
            else if constexpr (std::is_same_v<T, int>) {
                result.push_back(inTuple[i].I());
            }
            else if constexpr (std::is_same_v<T, std::string>) {
                result.push_back(std::string(inTuple[i].S()));
            }
            else {
                static_assert(always_false<T>, "Unsupported type for tupleToVector");
            }
        }
        return result;
    }

    inline HalconCpp::HTuple vectorToHTuple(const std::vector<std::string>& inVec) {
        HalconCpp::HTuple tuple;
        for (const auto& s : inVec)
            tuple.Append(s.c_str());
        return tuple;
    }
    inline cv::Mat HObjectToMat(const HalconCpp::HObject& hObj) {
        cv::Mat cvImg;
        Hlong width, height;
        HalconCpp::HTuple channels;

        HalconCpp::HImage hImg(hObj);
        hImg = hImg.ConvertImageType("byte");
        channels = hImg.CountChannels();

        if (channels[0].I() == 1) {
            HalconCpp::HString cType;
            unsigned char* ptr = (unsigned char*)hImg.GetImagePointer1(&cType, &width, &height);
            cvImg.create(height, width, CV_8UC1);
            memcpy(cvImg.data, ptr, width * height);
        }
        else if (channels[0].I() == 3) {
            HalconCpp::HString cType;
            Hlong width, height;
            void* ptrR, * ptrG, * ptrB;

            hImg.GetImagePointer3(&ptrR, &ptrG, &ptrB, &cType, &width, &height);
            int W = width, H = height;

            cv::Mat redPlane(H, W, CV_8UC1, ptrR);
            cv::Mat greenPlane(H, W, CV_8UC1, ptrG);
            cv::Mat bluePlane(H, W, CV_8UC1, ptrB);

            std::vector<cv::Mat> channelsVec = { bluePlane, greenPlane, redPlane };
            cv::merge(channelsVec, cvImg);
        }
        return cvImg;
    }

    inline HalconCpp::HObject MatToHObject(const cv::Mat& image) {
        HalconCpp::HObject hObj;
        int height = image.rows;
        int width = image.cols;

        if (image.type() == CV_8UC1) {
            uchar* data = image.data;
            HalconCpp::GenImage1(&hObj, "byte", width, height, (Hlong)data);
        }
        else if (image.type() == CV_8UC3) {
            std::vector<cv::Mat> planes;
            cv::split(image, planes);

            HalconCpp::GenImage3(&hObj, "byte", width, height,
                (Hlong)(planes[2].data),
                (Hlong)(planes[1].data),
                (Hlong)(planes[0].data));
        }
        return hObj;
    }

    inline cv::Mat VisualizeHObject(HalconCpp::HObject inHObj, cv::Size inSize) {
        HalconCpp::HImage tmpImg;
        tmpImg.GenImageConst("byte", inSize.width, inSize.height);
        
        HalconCpp::HImage ImageResult;
        ImageResult = tmpImg.PaintRegion(inHObj, 255, "fill");

        cv::Mat resImg = HObjectToMat(ImageResult);

        return resImg;
    }
}

#endif
