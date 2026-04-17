#pragma once
#ifndef CM_AFFINE_HPP
#define CM_AFFINE_HPP
#include <opencv2/opencv.hpp>


namespace cm {
    inline cv::Mat getTranslationTransformationMatrix(const cv::Point2d& inDeltaXY) {
        return (cv::Mat_<double>(3, 3) << 1, 0, inDeltaXY.x, 0, 1, inDeltaXY.y, 0, 0, 1);
    }

    inline cv::Mat getRotationTransformationMatrix(const double& inDeltaA) {
        const double T = inDeltaA / 180. * CV_PI;
        const double sinT = sin(T), cosT = cos(T);
        return (cv::Mat_<double>(3, 3) << cosT, -sinT, 0, sinT, cosT, 0, 0, 0, 1);
    }

    inline cv::Mat getScaleTransformationMatrix(const cv::Point2d& inSclFacXY) {
        return cv::Mat_<double>(3, 3) << inSclFacXY.x, 0, 0, 0, inSclFacXY.y, 0, 0, 0, 1;
    }

    inline cv::Mat getFlipHorizontalTransformationMatrix(const cv::Size& inImgSize) {
        return cv::Mat_<double>(3, 3) << -1, 0, inImgSize.width - 1, 0, 1, 0, 0, 0, 1;
    }

    inline cv::Mat getFlipVerticalTransformationMatrix(const cv::Size& inImgSize) {
        return cv::Mat_<double>(3, 3) << 1, 0, 0, 0, -1, inImgSize.height - 1, 0, 0, 1;
    }

    inline cv::Mat getFlipAllTransformationMatrix(const cv::Size& inImgSize) {
        return cv::Mat_<double>(3, 3) << -1, 0, inImgSize.width - 1, 0, -1, inImgSize.height - 1, 0, 0, 1;
    }

    inline cv::Mat getTranspositionTransformationMatrix() {
        return cv::Mat_<double>(3, 3) << 0, 1, 0, 1, 0, 0, 0, 0, 1;
    }

    inline cv::Mat getAffineMatrix(const cv::Mat& inAffineMat33) {
        return inAffineMat33(cv::Rect(0, 0, 3, 2)).clone();
    }
    

    inline cv::Mat getCtrRotImg(const cv::Mat& inSrcImg, const cv::Size& inImgSize, const double& inAngle) {
        const cv::Point2d srcCtr((inSrcImg.cols - 1) * 0.5, (inSrcImg.rows - 1) * 0.5);
        const cv::Mat transMat1 = getTranslationTransformationMatrix(-srcCtr);
        const cv::Mat rotateMat = getRotationTransformationMatrix(inAngle);
        const cv::Point2d distCtr((inImgSize.width - 1) * 0.5, (inImgSize.height - 1) * 0.5);
        const cv::Mat transMat2 = getTranslationTransformationMatrix(distCtr);
        const cv::Mat transMat = getAffineMatrix(transMat2 * rotateMat * transMat1);
        cv::Mat resMat;
        cv::warpAffine(inSrcImg, resMat, transMat, inImgSize);
        return resMat;
    }

    inline cv::Mat getCtrTraImg(const cv::Mat& inSrcImg, const cv::Size& inImgSize, const cv::Point2d& inDelta) {
        const cv::Point2d srcCtr((inSrcImg.cols - 1) * 0.5, (inSrcImg.rows - 1) * 0.5);
        const cv::Mat transMat1 = getTranslationTransformationMatrix(-srcCtr);
        const cv::Point2d distCtr((inImgSize.width - 1) * 0.5, (inImgSize.height - 1) * 0.5);
        const cv::Mat transMat2 = getTranslationTransformationMatrix(distCtr + inDelta);
        const cv::Mat transMat = getAffineMatrix(transMat2 * transMat1);
        cv::Mat resMat;
        cv::warpAffine(inSrcImg, resMat, transMat, inImgSize);
        return resMat;
    }

    inline void getCenterScalingAndRotationTransformationImage(const cv::Mat& inRotImg, const cv::Point2d& inSclFac, const cv::Point2d& inRotCtr, const double& inRotAng, cv::Mat& outResImg, const cv::Size& inResSize, const cv::Point2d& inResCtr, const int& inAffFlag = cv::INTER_LINEAR) {
        const cv::Mat transMat1 = getTranslationTransformationMatrix(-inRotCtr);
        const cv::Mat scaleMat = getScaleTransformationMatrix(inSclFac);
        const cv::Mat rotMat = getRotationTransformationMatrix(-inRotAng);
        const cv::Mat transMat2 = getTranslationTransformationMatrix(inResCtr);
        const cv::Mat transMat = getAffineMatrix(transMat2 * rotMat * scaleMat * transMat1);
        cv::warpAffine(inRotImg, outResImg, transMat, inResSize, inAffFlag);
    }

    inline cv::Mat getTRTImg(const cv::Mat& inSrcImg, const cv::Point2f& inT1, const float& inR, const cv::Point2f& inT2, const cv::Size& inSize, const int& inAffFlag = cv::INTER_LINEAR) {
        const cv::Mat tMat1 = getTranslationTransformationMatrix(inT1);
        const cv::Mat rMat = getRotationTransformationMatrix(inR);
        const cv::Mat tMat2 = getTranslationTransformationMatrix(inT2);
        const cv::Mat affMat = getAffineMatrix(tMat2 * rMat * tMat1);
        cv::Mat resImg;
        cv::warpAffine(inSrcImg, resImg, affMat, inSize, inAffFlag);
        return resImg;
    }
}

#endif
