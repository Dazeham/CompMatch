#include <ppl.h>
#include <random>
#include <numeric>
#include "cm_svg.hpp"
#include "cm_affine.hpp"
#include "cm_match_box.hpp"
#include "cm_intrinsics.hpp"
#include "cm_utils.hpp"


using namespace cm;

//////*****************    形状模板匹配    ******************//////
CTemplateShapeBoxType::CTemplateShapeBoxType(float inScaleX, float inScaleY) : CTemplatePartGroup(inScaleX, inScaleY) {
    // 元件参数
    mTotalX = 0;
    mTotalY = 0;

    // 匹配参数
    mStepNum = 0;
    mAngleRange = 3;

    // 额外匹配参数
    mLineModOffser = 3;
    mLineModAngle = 3;
    mLineUseRatio = 0.9;
    mLineRowBegRatio = 0.1;
    mLineMagTresh = 0.15;
    mLineDistTresh = 1;
}

AnswerType CTemplateShapeBoxType::GenerateTemplate(std::shared_ptr<Component> inCompPtr) {
    AnswerType answer = IMG_SUCCESS_ANS().SetErrCode();

    // 获取刻度
    //mScaleX = inScale.x;
    //mScaleY = inScale.y;
    const double minScale = std::min(mScaleX, mScaleY);
    mScaleFactor = cv::Point2d(mScaleX < mScaleY ? 1 : minScale / mScaleX, mScaleY < mScaleX ? 1 : minScale / mScaleY);

    // 获取箱形元件尺寸
    mTotalX = inCompPtr->GetCommonData().GetComponentLenth() / minScale;
    mTotalY = inCompPtr->GetCommonData().GetComponentWidth() / minScale;
    mVecLeadNum.resize(4, 0);	            //引脚数目
    mVecLeadLength.resize(4, 0);            //引脚长度
    mVecLeadWidth.resize(4, 0);	            //引脚宽度
    mVecCenterX.resize(4, 0);               //引脚组中心x
    mVecCenterY.resize(4, 0);               //引脚组中心y

    std::shared_ptr<cm::LeadGroupBasic> pLead = std::dynamic_pointer_cast<cm::LeadGroupBasic>(inCompPtr->GetOneAbstractConfigurationBasic(cm::ConfigurationBasicType::LEAD_GROUP));
    if (pLead != nullptr) {
        for (int i = 0; i < pLead->ParamCount(); ++i) {
            cm::LeadGroupParam leadParam = pLead->GetParam(i);
            auto it = mMapAngleToIdx.find(leadParam.GetShapeParam().angle);
            int idx = -1;
            if (it != mMapAngleToIdx.end()) {
                idx = it->second;
            }
            else {
                continue;
            }
    
            cm::LeadGroupParam::ShapeParam leadShapeParam = leadParam.GetShapeParam();
    
            mVecLeadNum[idx] = leadShapeParam.number;
            mVecLeadLength[idx] = leadShapeParam.length / minScale;
            mVecLeadWidth[idx] = leadShapeParam.width / minScale;
            mVecCenterX[idx] = leadShapeParam.center_x / minScale;
            mVecCenterY[idx] = -leadShapeParam.center_y / minScale;
        }
    }

    // 获取箱形元件姿态
    const int leadSideNum = std::accumulate(mVecLeadNum.begin(), mVecLeadNum.end(), 0);
    if (leadSideNum == 2 &&
        mVecLeadNum[0] == 1 && mVecLeadLength[0] > 0 && mVecLeadWidth[0] > 0 &&
        mVecLeadNum[2] == 1 && mVecLeadLength[2] > 0 && mVecLeadWidth[2] > 0) {
    }
    else if (leadSideNum == 2 &&
        mVecLeadNum[1] == 1 && mVecLeadLength[1] > 0 && mVecLeadWidth[1] > 0 &&
        mVecLeadNum[3] == 1 && mVecLeadLength[3] > 0 && mVecLeadWidth[3] > 0) {
    }
    else if (mTotalY > mTotalX) {
    }
    else {
    }

    // 绘制源模板 无缩放 无旋转
    float radius;
    GetRectCornerRadius(cv::Size2f(mTotalX, mTotalY), minScale, minScale, radius);
    mVecBoxSide = std::vector<int>(4, 0);
    std::vector<cv::Mat> rectSrcTpl;
    GetRectSourceTemplateS(cv::Size2f(mTotalX, mTotalY), mVecLeadNum, mVecLeadLength, mVecLeadWidth, mVecCenterX, mVecCenterY, radius, mVecBoxSide, rectSrcTpl, 1, 0);

    // 模板超限保护
    float baseSplStep = 1;
    GetBaseSampleStep(rectSrcTpl, baseSplStep);

    // 设置匹配参数
    //const float minAngle = std::max(atan2(1, std::max(mTotalX, mTotalY) * 0.5) / CV_PI * 180 * 0.33, 0.2);

    mStepNum = 3;
    //if ((mTotalX + mTotalY) < 65) {
    //    mPyramidLevels = { 1, 0, 0 };
    //    mStepPixels = { 1, 1, 1 };
    //    mSobelSizes = { 3, 3, 3 };
    //    mSampleSteps = { baseSplStep * float(1.), baseSplStep * float(1.), baseSplStep * float(1.) };
    //    // 额外匹配参数
    //    mLineModOffser = 3;
    //    mLineModAngle = 5;
    //    mLineUseRatio = 1.0;
    //    mLineRowBegRatio = 0.3;
    //    mLineMagTresh = 0.15;
    //    mLineDistTresh = 0.6;
    //}
    //else if ((mTotalX + mTotalY) < 105) {
    //    mPyramidLevels = { 1, 0, 0 };
    //    mStepPixels = { 2, 2, 1 };
    //    mSobelSizes = { 3, 3, 3 };
    //    mSampleSteps = { baseSplStep * float(1.), baseSplStep * float(1.), baseSplStep * float(1.) };
    //    // 额外匹配参数
    //    mLineModOffser = 3;
    //    mLineModAngle = 5;
    //    mLineUseRatio = 1.0;
    //    mLineRowBegRatio = 0.3;
    //    mLineMagTresh = 0.15;
    //    mLineDistTresh = 1;
    //}
    //else {
        mPyramidLevels = { 2, 1, 0 };
        mStepPixels = { 1, 1, 1 };
        mSobelSizes = { 3, 3, 3 };
        mSampleSteps = { baseSplStep * mSplItv, baseSplStep * mSplItv, baseSplStep * mSplItv };
        // 额外匹配参数
        mLineModOffser = 3;
        mLineModAngle = 5;
        mLineUseRatio = 0.9;
        mLineRowBegRatio = 0.1;
        mLineMagTresh = 0.15;
        mLineDistTresh = 1.25;
    //}
    //if (minAngle > 1) {
    //    mStepAngles[2] = 1;
    //}
    //mStepAngles = { 3, 0.5, 0.1 };
    GetStepAngles(cv::Size2d(mTotalX, mTotalY), mPyramidLevels, mStepAngles);
    mAngleRange = mStepAngles[0];

    // 绘制正模板
    mRectStepTemplates.resize(mStepNum);
    concurrency::parallel_for(0, mStepNum, [&](int stepNo) {
        GetRectSourceTemplateS(cv::Size2f(mTotalX, mTotalY), mVecLeadNum, mVecLeadLength, mVecLeadWidth, mVecCenterX, mVecCenterY, radius, mVecBoxSide, mRectStepTemplates[stepNo], mSampleSteps[stepNo], mPyramidLevels[stepNo]);
        });

    // 严格限制模板规模
    GetStrictSampleShapeTemplates(mRectStepTemplates, mRectStepTemplates);

    // 绘制旋转模板
    GetRotatedShapeTemplates(mRectStepTemplates[0], mRectRotTemplatesCoarse, mBeginAngle + mAngleRange, mEndAngle - mAngleRange, mStepAngles[0]);
    GetScaleShapeTemplates(mRectRotTemplatesCoarse, mScaleFactor);
    GetRotatedShapeTemplates(mRectStepTemplates[1], mRectRotTemplates, mBeginAngle, mEndAngle, mStepAngles[1]);
    GetScaleShapeTemplates(mRectRotTemplates, mScaleFactor);

    // 绘制多尺寸模板
    float tolerance;
    GetRectSizeTolerance(cv::Size2f(mTotalX, mTotalY), minScale, minScale, tolerance);
    tolerance *= 2. / 3.;
    
    std::vector<cv::Size2f> vecSize = {
        cv::Size2f(mTotalX, mTotalY),
    };
    
    mRectMultiScaleTemplates.resize(vecSize.size());
    concurrency::parallel_for(0, int(vecSize.size()), [&](int i) {
        GetRectSourceTemplateS(vecSize[i], mVecLeadNum, mVecLeadLength, mVecLeadWidth, mVecCenterX, mVecCenterY, radius, mVecBoxSide, mRectMultiScaleTemplates[i], mSampleSteps[2], mPyramidLevels[2]);
        });
    GetRectSourceTemplateR(cv::Size2f(mTotalX, mTotalY), mVecLeadNum, mVecLeadLength, mVecLeadWidth, mVecCenterX, mVecCenterY, radius, mVecBoxSide, mRectTemplatesRect, mSampleSteps[2], mPyramidLevels[2]);

    return answer;
}

AnswerType CTemplateShapeBoxType::TemplateMatch(const cv::Mat& inSrcImg, cv::Point2f& outOffset, float& outAngle, float& outScore) {
    AnswerType answer = IMG_SUCCESS_ANS().SetErrCode();

#if showTimeFlag
    std::cout << "||===  计时开始  ===||" << std::endl;  // 计时结束
    double time_start, tmpTime;
#endif

    // 判断是否已绘制模板
    if (mRectMultiScaleTemplates.size() == 0) {
        answer = IMG_PARAM_ERROR_ANS().SetErrCode(IMG_PARAM_ERROR_ANS::ANS2::TemplateNumberError);
        return answer;
    }

    // 截图
    mPartImg = inSrcImg;

    // 匹配
#if showTimeFlag
    time_start = cv::getTickCount();  // 计时开始
#endif
    cv::Point2f offset;
    float angle = 0.;
    float score = 0.;
    std::vector<cv::Mat> cropGradImgs;
    float maxMagVal;
    cv::Mat crsMagImg;
    answer = PartDetect(cv::Point2f(mScaleX, mScaleY), mPartImg, mRectStepTemplates, mRectRotTemplatesCoarse, mRectRotTemplates, mRectMultiScaleTemplates, mPyramidLevels, mStepPixels, mBeginAngle, mEndAngle, mAngleRange, mStepAngles, mSobelSizes, cv::Size2f(mTotalX, mTotalY), cv::Size2f(mTotalX, mTotalY), mStepNum, offset, angle, score, crsMagImg, mCropImg, cropGradImgs, mCropMagImg, maxMagVal, mLeftTop, mScaleFactor);
    if (answer.first != ALGErrCode::IMG_SUCCESS) {
        return answer;
    }
#if showTimeFlag
    tmpTime = (cv::getTickCount() - time_start) * 1000. / double(cv::getTickFrequency());
    std::cout << " - 匹配: " << tmpTime << "ms" << std::endl;  // 计时结束
#endif


    // 箱形元件额外精度模块
#if showTimeFlag
    time_start = cv::getTickCount();  // 计时开始
#endif
    mSrcImgCtr = cv::Point2f((mPartImg.size().width - 1) * 0.5, (mPartImg.size().height - 1) * 0.5);
    mCropImgCtr = cv::Point2f((mCropImg.cols - 1) * 0.5, (mCropImg.rows - 1) * 0.5);
    const cv::Point2f inScale(mScaleX, mScaleY);
    mScaleFactorInverse = cv::Point2d(inScale.x < inScale.y ? 1 : inScale.x / inScale.y, inScale.y < inScale.x ? 1 : inScale.y / inScale.x);
    // 亚像素方法 开始
    ///*
    answer = GetPreciseRectPosition(cv::Point2f(mScaleX, mScaleY), mPartImg.size(), mCropImg, mCropMagImg, cropGradImgs, mLeftTop, cv::Size2d(mTotalX, mTotalY), offset, angle, mVecBoxSide, mLineModOffser, mLineModAngle, mLineUseRatio, mLineRowBegRatio, mLineMagTresh, mLineDistTresh);
    //answer = GetPrecisePosition(mSrcImgCtr, mScaleFactor, mRectTemplatesRect, cv::Point2f(mScaleX, mScaleY), mPartImg.size(), mCropImg, mCropMagImg, cropGradImgs, mLeftTop, cv::Size2d(mTotalX, mTotalY), offset, angle);
    if (answer.first == ALGErrCode::IMG_SUCCESS) {
        std::vector<cv::Mat> tmpRotTpl;
        GetRotatedShapeTemplate(mRectMultiScaleTemplates[0], tmpRotTpl, angle);
        GetScaleShapeTemplate(tmpRotTpl, mScaleFactor);
        const cv::Point2f imgCenter((mPartImg.cols - 1) * 0.5, (mPartImg.rows - 1) * 0.5);
        cv::Point2f rOffset(offset.x * pow(0.5, mPyramidLevels[2]) - mLeftTop.x + imgCenter.x, offset.y * pow(0.5, mPyramidLevels[2]) - mLeftTop.y + imgCenter.y);
        score = cm::calculateGradientScore(cropGradImgs[0], cropGradImgs[1], tmpRotTpl[0], tmpRotTpl[1], tmpRotTpl[2], tmpRotTpl[3], rOffset.x, rOffset.y) * 100;
#ifdef _DEBUG
        CheckMatchTemplate(mPartImg, mRectMultiScaleTemplates[0], offset, angle, mPyramidLevels[2], 0, mScaleFactor);
        CheckMatchTemplateSVG(mPartImg, mRectMultiScaleTemplates[0], offset, angle, mPyramidLevels[2], 0, mScaleFactor);
#endif
    }
    else {
        answer = IMG_SUCCESS_ANS().SetErrCode();
    }
    //*/
    // 亚像素方法 结束
#if showTimeFlag
    tmpTime = (cv::getTickCount() - time_start) * 1000. / double(cv::getTickFrequency());
    std::cout << " - 精度补偿: " << tmpTime << "ms" << std::endl;  // 计时结束
#endif

    // 结果输出
    outOffset = offset;
    outAngle = angle;
    outScore = 50 + score * 0.5;

    return answer;
}

AnswerType CTemplateShapeBoxType::SaveResult(const cv::Mat& inSrcImg, const cv::Point2f& inOffset, const float& inAngle, const float& inScore, const AnswerType& inAnswer, const double& inTime, const std::string& inPath) {
    AnswerType answer = IMG_SUCCESS_ANS().SetErrCode();

    std::vector<cv::Mat> inSrcTpl = mRectMultiScaleTemplates[0];
    cv::Point2d inSclFac = mScaleFactor;
    int inPyrLvl = mPyramidLevels[2];
    int inMinPyrLvl = 0;

    int showImgSize = 60;
    int bodWidth = (inSrcImg.cols - showImgSize) * 0.5;
    //cv::Mat showImg = inSrcImg(cv::Rect(bodWidth, bodWidth, showImgSize, showImgSize)).clone();
    cv::Mat showImg = inSrcImg;
    //if (showImg.depth() != CV_32F) {
    //    showImg.convertTo(showImg, CV_32F);
    //}
    //if (showImg.channels() != 3) {
    //    cv::Mat c1 = showImg.clone();
    //    cv::Mat c2 = showImg.clone();
    //    cv::Mat c3 = showImg.clone();
    //    std::vector<cv::Mat> cs = { c1, c2, c3 };
    //    cv::merge(cs, showImg);
    //}

    std::vector<cv::Mat> rotTpl;
    GetRotatedShapeTemplate(inSrcTpl, rotTpl, inAngle);
    GetScaleShapeTemplate(rotTpl, inSclFac);

    cv::Point2f imgCenter((showImg.cols - 1) * 0.5, (showImg.rows - 1) * 0.5);
    cv::Point2f offset(imgCenter.x + inOffset.x * powf(0.5, inPyrLvl - inMinPyrLvl), imgCenter.y + inOffset.y * powf(0.5, inPyrLvl - inMinPyrLvl));

    //// 梯度模板
    //for (int i = 0; i < rotTpl[0].cols; i++) {
    //    int row = cvRound(rotTpl[1].ptr<float>()[i] + offset.y);
    //    int col = cvRound(rotTpl[0].ptr<float>()[i] + offset.x);
    //    if (row < 0 || row >= showImg.rows || col < 0 || col >= showImg.cols) {
    //        continue;
    //    }
    //    cv::Vec3f& pixel = showImg.at<cv::Vec3f>(row, col);
    //    pixel[0] = 255;
    //    pixel[1] = 0;
    //    pixel[2] = 0;
    //}

    //// 灰度模板
    //if (inSrcTpl.size() == 6) {
    //    for (int i = 0; i < rotTpl[4].cols; i++) {
    //        int row = cvRound(rotTpl[5].ptr<float>()[i] + offset.y);
    //        int col = cvRound(rotTpl[4].ptr<float>()[i] + offset.x);
    //        if (row < 0 || row >= showImg.rows || col < 0 || col >= showImg.cols) {
    //            continue;
    //        }
    //        cv::Vec3f& pixel = showImg.at<cv::Vec3f>(row, col);
    //        pixel[0] = 0;
    //        pixel[1] = 255;
    //        pixel[2] = 0;
    //    }
    //}

    // 保存SVG
    cv::Mat magImg;
    std::vector<cv::Mat> gradImgs;
    cm::getNormalizedGradientAndMagnitudeImages(showImg, gradImgs, magImg, true);
    //GetMaxBlurImage(magImg, magImg);

    const int roiWidth = 120;
    const int roiHeight = roiWidth;
    const int borWidth = (inSrcImg.cols - roiWidth) / 2;
    const int borHeight = (inSrcImg.rows - roiHeight) / 2;
    cv::Mat roiImg = inSrcImg(cv::Rect(borWidth, borHeight, roiWidth, roiHeight)).clone();
    cv::Point2f imgOfs((inSrcImg.cols - roiImg.cols) * 0.5, (inSrcImg.rows - roiImg.rows) * 0.5);
    SVGTool st(inPath, roiImg);

    // 显示点
    //st.drawCircle(cv::Point2f(0.5, 0.5), 0.5, "red");  // 原点
    //for (int i = 0; i < rotTpl[0].cols; i++) {  // 梯度模板
    //    float row = rotTpl[1].ptr<float>()[i] + offset.y;
    //    float col = rotTpl[0].ptr<float>()[i] + offset.x;
    //    if (row < 0 || row >= showImg.rows || col < 0 || col >= showImg.cols) {
    //        continue;
    //    }
    //    cv::Point2f centerPt(col, row);
    //    float radius = 0.6f;
    //    st.drawCircle(centerPt - imgOfs + cv::Point2f(0.5, 0.5), radius, "#C44DFF");
    //}
    //if (rotTpl.size() == 6) {  // 灰度模板
    //    if (rotTpl[4].cols > 0) {
    //        for (int i = 0; i < rotTpl[4].cols; i++) {
    //            float row = rotTpl[5].ptr<float>()[i] + offset.y;
    //            float col = rotTpl[4].ptr<float>()[i] + offset.x;
    //            if (row < 0 || row >= showImg.rows || col < 0 || col >= showImg.cols) {
    //                continue;
    //            }
    //            cv::Point2f centerPt(col, row);
    //            float radius = 0.6f;
    //            st.drawCircle(centerPt - imgOfs + cv::Point2f(0.5, 0.5), radius, "#FF8C00");
    //        }
    //    }
    //}

    // 显示轮廓线
    st.drawRect(offset - imgOfs + cv::Point2f(0.5, 0.5), cv::Size2f(mTotalX, mTotalY), inAngle ,1, "red");
    float score = 0;
    if (inAnswer.first == ALGErrCode::IMG_SUCCESS) {
        score = inScore;
    }
    else {
        st.drawLine(cv::Point(1, 1), cv::Point(showImg.cols-1, showImg.rows-1), 2, "red");
        st.drawLine(cv::Point(showImg.cols-1, 1), cv::Point(1, showImg.rows-1), 2, "red");
    }

    //// 显示分数
    //int fontSize = showImg.rows * 0.1;
    //st.drawText(cv::Point2f(1, fontSize), "red", std::to_string(fontSize) + "px", "Time: " + std::to_string(inTime) + " ms");
    //st.drawText(cv::Point2f(1, showImg.rows-1), "red", std::to_string(fontSize) + "px", "Score: " + std::to_string(score));

    st.close();

    return answer;
}

// 模板绘制
AnswerType CTemplateShapeBoxType::GetRectCornerRadius(const cv::Size2f& inRectSize, const double& inScaleX, const double& inScaleY, float& outRad) {
    AnswerType answer = IMG_SUCCESS_ANS().SetErrCode();

    float tmpRadius = std::max(inRectSize.width, inRectSize.height) * 0.04;
    float maxRadius = 0.15 / std::min(inScaleX, inScaleY);
    outRad = std::min(tmpRadius, maxRadius);

    return answer;
}

AnswerType CTemplateShapeBoxType::GetRectSourceTemplate(const cv::Size2f& inRectSize, const float& inCorRad, const std::vector<int>& inVecBoxSide, std::vector<cv::Mat>& outSrcTpl, const float& inSplStep) {
    AnswerType answer = IMG_SUCCESS_ANS().SetErrCode();

    // 计算周长
    const float perimeter = 2 * CV_PI * inCorRad + 2 * (inRectSize.width + inRectSize.height) - 8 * inCorRad;
    const int ptNum = floorf(perimeter / inSplStep) + 1;
    //if (ptNum < 4)

    // 采样
    cv::Mat tX = cv::Mat::zeros(cv::Size(1, std::floor(ptNum)), CV_32FC1);
    cv::Mat tY = cv::Mat::zeros(cv::Size(1, std::floor(ptNum)), CV_32FC1);
    cv::Mat tGX = cv::Mat::zeros(cv::Size(1, std::floor(ptNum)), CV_32FC1);
    cv::Mat tGY = cv::Mat::zeros(cv::Size(1, std::floor(ptNum)), CV_32FC1);
    for (int ptNo = 0; ptNo < ptNum; ++ptNo) {
        float distance = ptNo * inSplStep;

        if ((distance < 0.5 * CV_PI * inCorRad) && (inVecBoxSide[0] == 0 && inVecBoxSide[1] == 0)) {
            float angle = distance / (0.5 * CV_PI * inCorRad) * 90 * CV_PI / 180.; // 弧度制
            tX.ptr<float>(ptNo)[0] = (-0.5 * inRectSize.width + inCorRad) - (inCorRad * cosf(angle));
            tY.ptr<float>(ptNo)[0] = (-0.5 * inRectSize.height + inCorRad) - (inCorRad * sinf(angle));
            tGX.ptr<float>(ptNo)[0] = cosf(angle);
            tGY.ptr<float>(ptNo)[0] = sinf(angle);
        }
        else if ((distance < (0.5 * CV_PI * inCorRad + inRectSize.width - 2 * inCorRad)) && (inVecBoxSide[0] == 0)) {
            tX.ptr<float>(ptNo)[0] = (-0.5 * inRectSize.width + inCorRad) + (distance - 0.5 * CV_PI * inCorRad);
            tY.ptr<float>(ptNo)[0] = (-0.5 * inRectSize.height);
            tGX.ptr<float>(ptNo)[0] = 0;
            tGY.ptr<float>(ptNo)[0] = 1;
        }
        else if ((distance < (1.0 * CV_PI * inCorRad + inRectSize.width - 2 * inCorRad)) && (inVecBoxSide[0] == 0 && inVecBoxSide[3] == 0)) {
            float angle = (distance - (0.5 * CV_PI * inCorRad + inRectSize.width - 2 * inCorRad)) / (0.5 * CV_PI * inCorRad) * 90 * CV_PI / 180.; // 弧度制
            tX.ptr<float>(ptNo)[0] = (0.5 * inRectSize.width - inCorRad) + (inCorRad * sinf(angle));
            tY.ptr<float>(ptNo)[0] = (-0.5 * inRectSize.height + inCorRad) - (inCorRad * cosf(angle));
            tGX.ptr<float>(ptNo)[0] = -sinf(angle);
            tGY.ptr<float>(ptNo)[0] = cosf(angle);
        }
        else if ((distance < (1.0 * CV_PI * inCorRad + inRectSize.width + inRectSize.height - 4 * inCorRad)) && (inVecBoxSide[3] == 0)) {
            tX.ptr<float>(ptNo)[0] = (0.5 * inRectSize.width);
            tY.ptr<float>(ptNo)[0] = (-0.5 * inRectSize.height + inCorRad) + (distance - (1.0 * CV_PI * inCorRad + inRectSize.width - 2 * inCorRad));
            tGX.ptr<float>(ptNo)[0] = -1;
            tGY.ptr<float>(ptNo)[0] = 0;
        }
        else if (((distance < (1.5 * CV_PI * inCorRad + inRectSize.width + inRectSize.height - 4 * inCorRad)) && (inVecBoxSide[2] == 0 && inVecBoxSide[3] == 0)) && (inVecBoxSide[2] == 0 && inVecBoxSide[3] == 0)) {
            float angle = (distance - (1.0 * CV_PI * inCorRad + inRectSize.width + inRectSize.height - 4 * inCorRad)) / (0.5 * CV_PI * inCorRad) * 90 * CV_PI / 180.; // 弧度制
            tX.ptr<float>(ptNo)[0] = (0.5 * inRectSize.width - inCorRad) + (inCorRad * cosf(angle));
            tY.ptr<float>(ptNo)[0] = (0.5 * inRectSize.height - inCorRad) + (inCorRad * sinf(angle));
            tGX.ptr<float>(ptNo)[0] = -cosf(angle);
            tGY.ptr<float>(ptNo)[0] = -sinf(angle);
        }
        else if ((distance < (1.5 * CV_PI * inCorRad + 2 * inRectSize.width + inRectSize.height - 6 * inCorRad)) && (inVecBoxSide[2] == 0)) {
            tX.ptr<float>(ptNo)[0] = (0.5 * inRectSize.width - inCorRad) - (distance - (1.5 * CV_PI * inCorRad + inRectSize.width + inRectSize.height - 4 * inCorRad));
            tY.ptr<float>(ptNo)[0] = (0.5 * inRectSize.height);
            tGX.ptr<float>(ptNo)[0] = 0;
            tGY.ptr<float>(ptNo)[0] = -1;
        }
        else if ((distance < (2.0 * CV_PI * inCorRad + 2 * inRectSize.width + inRectSize.height - 6 * inCorRad)) && (inVecBoxSide[1] == 0 && inVecBoxSide[2] == 0)) {
            float angle = (distance - (1.5 * CV_PI * inCorRad + 2 * inRectSize.width + inRectSize.height - 6 * inCorRad)) / (0.5 * CV_PI * inCorRad) * 90 * CV_PI / 180.; // 弧度制
            tX.ptr<float>(ptNo)[0] = (-0.5 * inRectSize.width + inCorRad) - (inCorRad * sinf(angle));
            tY.ptr<float>(ptNo)[0] = (0.5 * inRectSize.height - inCorRad) + (inCorRad * cosf(angle));
            tGX.ptr<float>(ptNo)[0] = sinf(angle);
            tGY.ptr<float>(ptNo)[0] = -cosf(angle);
        }
        else if (inVecBoxSide[1] == 0) {
            tX.ptr<float>(ptNo)[0] = (-0.5 * inRectSize.width);
            tY.ptr<float>(ptNo)[0] = (0.5 * inRectSize.height - inCorRad) - (distance - (2.0 * CV_PI * inCorRad + 2 * inRectSize.width + inRectSize.height - 6 * inCorRad));
            tGX.ptr<float>(ptNo)[0] = 1;
            tGY.ptr<float>(ptNo)[0] = 0;
        }
    }

    outSrcTpl = { tX, tY, tGX, tGY };

    return answer;
}

AnswerType CTemplateShapeBoxType::GetRectSourceTemplateX(const cv::Size2f& inRectSize, const float& inCorRad, const std::vector<int>& inVecBoxSide, std::vector<cv::Mat>& outSrcTpl, const float& inSplStep) {
    AnswerType answer = IMG_SUCCESS_ANS().SetErrCode();

    // 计算周长
    const float perimeter = 2 * CV_PI * inCorRad + 2 * (inRectSize.width + inRectSize.height) - 8 * inCorRad;
    const int ptNum = floorf(perimeter / inSplStep) + 1;
    //if (ptNum < 4)

    // 删除圆角
    const std::vector<int> deleteFlag = { 0, 0, 0, 0, 0, 0, 0, 0 };

    // 采样
    std::vector<float> vecX, vecY, vecGX, vecGY;
    for (int ptNo = 0; ptNo < ptNum; ++ptNo) {
        float distance = ptNo * inSplStep;

        if (distance >= 0 &&
            distance < (0.5 * CV_PI * inCorRad) &&
            inVecBoxSide[0] == 0 && inVecBoxSide[1] == 0 && deleteFlag[0] == 0)
        {
            float angle = distance / (0.5 * CV_PI * inCorRad) * 90 * CV_PI / 180.; // 弧度制
            vecX.push_back((-0.5 * inRectSize.width + inCorRad) - (inCorRad * cosf(angle)));
            vecY.push_back((-0.5 * inRectSize.height + inCorRad) - (inCorRad * sinf(angle)));
            vecGX.push_back(cosf(angle));
            vecGY.push_back(sinf(angle));
        }
        else if (distance >= (0.5 * CV_PI * inCorRad) &&
            distance < (0.5 * CV_PI * inCorRad + inRectSize.width - 2 * inCorRad) &&
            inVecBoxSide[0] == 0 && deleteFlag[1] == 0)
        {
            vecX.push_back((-0.5 * inRectSize.width + inCorRad) + (distance - 0.5 * CV_PI * inCorRad));
            vecY.push_back((-0.5 * inRectSize.height));
            vecGX.push_back(0);
            vecGY.push_back(1);
        }
        else if (distance >= (0.5 * CV_PI * inCorRad + inRectSize.width - 2 * inCorRad) &&
            distance < (1.0 * CV_PI * inCorRad + inRectSize.width - 2 * inCorRad) &&
            inVecBoxSide[0] == 0 && inVecBoxSide[3] == 0 && deleteFlag[2] == 0)
        {
            float angle = (distance - (0.5 * CV_PI * inCorRad + inRectSize.width - 2 * inCorRad)) / (0.5 * CV_PI * inCorRad) * 90 * CV_PI / 180.; // 弧度制
            vecX.push_back((0.5 * inRectSize.width - inCorRad) + (inCorRad * sinf(angle)));
            vecY.push_back((-0.5 * inRectSize.height + inCorRad) - (inCorRad * cosf(angle)));
            vecGX.push_back(-sinf(angle));
            vecGY.push_back(cosf(angle));
        }
        else if (distance >= (1.0 * CV_PI * inCorRad + inRectSize.width - 2 * inCorRad) &&
            distance < (1.0 * CV_PI * inCorRad + inRectSize.width + inRectSize.height - 4 * inCorRad) &&
            inVecBoxSide[3] == 0 && deleteFlag[3] == 0)
        {
            vecX.push_back((0.5 * inRectSize.width));
            vecY.push_back((-0.5 * inRectSize.height + inCorRad) + (distance - (1.0 * CV_PI * inCorRad + inRectSize.width - 2 * inCorRad)));
            vecGX.push_back(-1);
            vecGY.push_back(0);
        }
        else if (distance >= (1.0 * CV_PI * inCorRad + inRectSize.width + inRectSize.height - 4 * inCorRad) &&
            distance < (1.5 * CV_PI * inCorRad + inRectSize.width + inRectSize.height - 4 * inCorRad) &&
            inVecBoxSide[2] == 0 && inVecBoxSide[3] == 0 && deleteFlag[4] == 0)
        {
            float angle = (distance - (1.0 * CV_PI * inCorRad + inRectSize.width + inRectSize.height - 4 * inCorRad)) / (0.5 * CV_PI * inCorRad) * 90 * CV_PI / 180.; // 弧度制
            vecX.push_back((0.5 * inRectSize.width - inCorRad) + (inCorRad * cosf(angle)));
            vecY.push_back((0.5 * inRectSize.height - inCorRad) + (inCorRad * sinf(angle)));
            vecGX.push_back(-cosf(angle));
            vecGY.push_back(-sinf(angle));
        }
        else if (distance >= (1.5 * CV_PI * inCorRad + inRectSize.width + inRectSize.height - 4 * inCorRad) &&
            distance < (1.5 * CV_PI * inCorRad + 2 * inRectSize.width + inRectSize.height - 6 * inCorRad) &&
            inVecBoxSide[2] == 0 && deleteFlag[5] == 0)
        {
            vecX.push_back((0.5 * inRectSize.width - inCorRad) - (distance - (1.5 * CV_PI * inCorRad + inRectSize.width + inRectSize.height - 4 * inCorRad)));
            vecY.push_back((0.5 * inRectSize.height));
            vecGX.push_back(0);
            vecGY.push_back(-1);
        }
        else if (distance >= (1.5 * CV_PI * inCorRad + 2 * inRectSize.width + inRectSize.height - 6 * inCorRad) &&
            distance < (2.0 * CV_PI * inCorRad + 2 * inRectSize.width + inRectSize.height - 6 * inCorRad) &&
            inVecBoxSide[1] == 0 && inVecBoxSide[2] == 0 && deleteFlag[6] == 0)
        {
            float angle = (distance - (1.5 * CV_PI * inCorRad + 2 * inRectSize.width + inRectSize.height - 6 * inCorRad)) / (0.5 * CV_PI * inCorRad) * 90 * CV_PI / 180.; // 弧度制
            vecX.push_back((-0.5 * inRectSize.width + inCorRad) - (inCorRad * sinf(angle)));
            vecY.push_back((0.5 * inRectSize.height - inCorRad) + (inCorRad * cosf(angle)));
            vecGX.push_back(sinf(angle));
            vecGY.push_back(-cosf(angle));
        }
        else if (distance >= (2.0 * CV_PI * inCorRad + 2 * inRectSize.width + inRectSize.height - 6 * inCorRad) &&
            inVecBoxSide[1] == 0 && deleteFlag[7] == 0)
        {
            vecX.push_back((-0.5 * inRectSize.width));
            vecY.push_back((0.5 * inRectSize.height - inCorRad) - (distance - (2.0 * CV_PI * inCorRad + 2 * inRectSize.width + inRectSize.height - 6 * inCorRad)));
            vecGX.push_back(1);
            vecGY.push_back(0);
        }
    }

    const int tplSize = vecX.size();
    cv::Mat tX = cv::Mat::zeros(cv::Size(1, std::floor(tplSize)), CV_32FC1);
    cv::Mat tY = cv::Mat::zeros(cv::Size(1, std::floor(tplSize)), CV_32FC1);
    cv::Mat tGX = cv::Mat::zeros(cv::Size(1, std::floor(tplSize)), CV_32FC1);
    cv::Mat tGY = cv::Mat::zeros(cv::Size(1, std::floor(tplSize)), CV_32FC1);

    for (int ptNo = 0; ptNo < tplSize; ++ptNo) {
        tX.ptr<float>(ptNo)[0] = vecX[ptNo];
        tY.ptr<float>(ptNo)[0] = vecY[ptNo];
        tGX.ptr<float>(ptNo)[0] = vecGX[ptNo];
        tGY.ptr<float>(ptNo)[0] = vecGY[ptNo];
    }

    outSrcTpl = { tX, tY, tGX, tGY };

#ifdef _DEBUG
    CheckShapeTemplates(outSrcTpl);
#endif // _DEBUG


    return answer;
}

AnswerType CTemplateShapeBoxType::GetRectLeadTemplate(const cv::Size2f& inRectLeadSize, const float& inCordRad, std::vector<cv::Mat>& outRectLeadTpl, const float& inSplStep) {
    AnswerType answer = IMG_SUCCESS_ANS().SetErrCode();

    const int ptNumX = std::max(int((inRectLeadSize.width - inCordRad * 2) / inSplStep + 1), 2);
    const float splStepX = (inRectLeadSize.width - inCordRad * 2) / float(ptNumX - 1);
    const int ptNumY = std::max(int((inRectLeadSize.height - inCordRad) / inSplStep + 1), 2);
    const float splStepY = (inRectLeadSize.height - inCordRad) / float(ptNumY - 1);

    const float drawRatio = 0.6;
    const float lengthRatio = drawRatio * 0.5;
    const float deltaP = 1;
    const int ptNumAX = std::max(int((inRectLeadSize.width - inCordRad * 2) * lengthRatio / inSplStep + 1), 2);
    const float splStepAX = (inRectLeadSize.width - inCordRad * 2) * lengthRatio / float(ptNumAX - 1);
    const int ptNumAY = std::max(int((inRectLeadSize.height - inCordRad) * drawRatio / inSplStep + 1), 2);
    const float splStepAY = (inRectLeadSize.height - inCordRad) * drawRatio / float(ptNumAY - 1);

    const int totalPtNum = ptNumY * 2 + ptNumAY * 4 + ptNumX + ptNumAX * 4;
    cv::Mat tX = cv::Mat::zeros(cv::Size(totalPtNum, 1), CV_32FC1);
    cv::Mat tY = cv::Mat::zeros(cv::Size(totalPtNum, 1), CV_32FC1);
    cv::Mat tGX = cv::Mat::zeros(cv::Size(totalPtNum, 1), CV_32FC1);
    cv::Mat tGY = cv::Mat::zeros(cv::Size(totalPtNum, 1), CV_32FC1);
    float* pX = tX.ptr<float>();
    float* pY = tY.ptr<float>();
    float* pGX = tGX.ptr<float>();
    float* pGY = tGY.ptr<float>();
    int idx = -1;
    for (int ptNo = 0; ptNo < ptNumY; ++ptNo) {  // 左
        ++idx;
        pX[idx] = -inRectLeadSize.width * 0.5;
        pY[idx] = inRectLeadSize.height - ptNo * splStepY;
        pGX[idx] = 1;
        pGY[idx] = 0;
    }
    for (int ptNo = 0; ptNo < ptNumAY; ++ptNo) {
        ++idx;
        pX[idx] = -inRectLeadSize.width * 0.5 - deltaP;
        pY[idx] = inRectLeadSize.height - ptNo * splStepAY;
        pGX[idx] = 1;
        pGY[idx] = 0;
    }
    for (int ptNo = 0; ptNo < ptNumAY; ++ptNo) {
        ++idx;
        pX[idx] = -inRectLeadSize.width * 0.5 + deltaP;
        pY[idx] = inRectLeadSize.height - ptNo * splStepAY;
        pGX[idx] = 1;
        pGY[idx] = 0;
    }
    for (int ptNo = 0; ptNo < ptNumY; ++ptNo) {  // 右
        ++idx;
        pX[idx] = inRectLeadSize.width * 0.5;
        pY[idx] = inRectLeadSize.height - ptNo * splStepY;
        pGX[idx] = -1;
        pGY[idx] = 0;
    }
    for (int ptNo = 0; ptNo < ptNumAY; ++ptNo) {
        ++idx;
        pX[idx] = inRectLeadSize.width * 0.5 + deltaP;
        pY[idx] = inRectLeadSize.height - ptNo * splStepAY;
        pGX[idx] = -1;
        pGY[idx] = 0;
    }
    for (int ptNo = 0; ptNo < ptNumAY; ++ptNo) {
        ++idx;
        pX[idx] = inRectLeadSize.width * 0.5 - deltaP;
        pY[idx] = inRectLeadSize.height - ptNo * splStepAY;
        pGX[idx] = -1;
        pGY[idx] = 0;
    }
    for (int ptNo = 0; ptNo < ptNumX; ++ptNo) {  // 上
        ++idx;
        pX[idx] = -inRectLeadSize.width * 0.5 + inCordRad + ptNo * splStepX;
        pY[idx] = 0;
        pGX[idx] = 0;
        pGY[idx] = 1;
    }
    for (int ptNo = 0; ptNo < ptNumAX; ++ptNo) {
        ++idx;
        pX[idx] = -inRectLeadSize.width * 0.5 + inCordRad + ptNo * splStepAX;
        pY[idx] = -deltaP;
        pGX[idx] = 0;
        pGY[idx] = 1;
    }
    for (int ptNo = 0; ptNo < ptNumAX; ++ptNo) {
        ++idx;
        pX[idx] = inRectLeadSize.width * 0.5 - inCordRad - ptNo * splStepAX;
        pY[idx] = -deltaP;
        pGX[idx] = 0;
        pGY[idx] = 1;
    }
    for (int ptNo = 0; ptNo < ptNumAX; ++ptNo) {
        ++idx;
        pX[idx] = -inRectLeadSize.width * 0.5 + inCordRad + ptNo * splStepAX;
        pY[idx] = deltaP;
        pGX[idx] = 0;
        pGY[idx] = 1;
    }
    for (int ptNo = 0; ptNo < ptNumAX; ++ptNo) {
        ++idx;
        pX[idx] = inRectLeadSize.width * 0.5 - inCordRad - ptNo * splStepAX;
        pY[idx] = deltaP;
        pGX[idx] = 0;
        pGY[idx] = 1;
    }


    cv::Mat tVX, tVY;
    ///*
    const float useRatio = 0.8;
    const float backboneWidth = inRectLeadSize.width * useRatio;
    const int ptNumVX = std::max(int(backboneWidth / inSplStep + 1), 2);
    const float splStepVX = (0.8 * inRectLeadSize.width) / float(ptNumX - 1);

    tVX = cv::Mat::zeros(cv::Size(ptNumVX, 1), CV_32FC1);
    tVY = cv::Mat::zeros(cv::Size(ptNumVX, 1), CV_32FC1);
    float* pVX = tVX.ptr<float>();
    float* pVY = tVY.ptr<float>();
    for (int ptNo = 0; ptNo < ptNumVX; ++ptNo) {
        pVX[ptNo] = -0.4 * inRectLeadSize.width + ptNo * splStepVX;
        pVY[ptNo] = 0.5 * inRectLeadSize.height;
    }
    //*/


    if (tVX.cols > 0) {
        outRectLeadTpl = { tX, tY, tGX, tGY, tVX, tVY };
    }
    else {
        outRectLeadTpl = { tX, tY, tGX, tGY };
    }

    return answer;
}

AnswerType CTemplateShapeBoxType::GetRectSourceTemplateR(const cv::Size2f& inRectSize, const std::vector<int>& inVecLeadNum, const std::vector<double>& inVecLeadLength, const std::vector<double>& inVecLeadWidth, std::vector<double>& inVecCenterX, std::vector<double> inVecCenterY, const float& inCorRad, const std::vector<int>& inVecBoxSide, std::vector<cv::Mat>& outSrcTpl, const float& inSplStep, const int& inPyrLvl) {
    AnswerType answer = IMG_SUCCESS_ANS().SetErrCode();

    cv::Size2f rectSize = inRectSize * powf(0.5, inPyrLvl);
    float corRad = inCorRad * powf(0.5, inPyrLvl);
    std::vector<int> vecLeadNum(inVecLeadNum.begin(), inVecLeadNum.end());
    std::vector<double> vecLeadLength;
    std::vector<double> vecLeadWidth;
    std::vector<double> vecCenterX;
    std::vector<double> vecCenterY;
    for (int i = 0; i < vecLeadNum.size(); ++i) {
        vecLeadLength.push_back(inVecLeadLength[i] * powf(0.5, inPyrLvl));
        vecLeadWidth.push_back(inVecLeadWidth[i] * powf(0.5, inPyrLvl));
        vecCenterX.push_back(inVecCenterX[i] * powf(0.5, inPyrLvl));
        vecCenterY.push_back(inVecCenterY[i] * powf(0.5, inPyrLvl));
    }


    cv::Mat tX, tY, tGX, tGY, tVX, tVY;
    const int leadNum = std::accumulate(vecLeadNum.begin(), vecLeadNum.end(), 0);
    const int ignNum = std::accumulate(inVecBoxSide.begin(), inVecBoxSide.end(), 0);
    
    if (true) {  // 纯矩形
        const float perimeter = 2 * (rectSize.width + rectSize.height);
        const int ptNum = floorf(perimeter / inSplStep) + 1;
        //if (ptNum < 4)

        // 采样
        tX = cv::Mat::zeros(cv::Size(std::floor(ptNum), 1), CV_32FC1);
        tY = cv::Mat::zeros(cv::Size(std::floor(ptNum), 1), CV_32FC1);
        tGX = cv::Mat::zeros(cv::Size(std::floor(ptNum), 1), CV_32FC1);
        tGY = cv::Mat::zeros(cv::Size(std::floor(ptNum), 1), CV_32FC1);
        for (int ptNo = 0; ptNo < ptNum; ++ptNo) {
            float distance = ptNo * inSplStep;

            if (distance < rectSize.width && inVecBoxSide[0] == 0) {
                tX.ptr<float>()[ptNo] = -0.5 * rectSize.width + distance;
                tY.ptr<float>()[ptNo] = -0.5 * rectSize.height;
                tGX.ptr<float>()[ptNo] = 0;
                tGY.ptr<float>()[ptNo] = 1;
            }
            else if (distance < rectSize.width + rectSize.height && inVecBoxSide[1] == 0) {
                tX.ptr<float>()[ptNo] = 0.5 * rectSize.width;
                tY.ptr<float>()[ptNo] = -0.5 * rectSize.height + (distance - rectSize.width);
                tGX.ptr<float>()[ptNo] = -1;
                tGY.ptr<float>()[ptNo] = 0;
            }
            else if (distance < (rectSize.width * 2 + rectSize.height) && inVecBoxSide[2] == 0) {
                tX.ptr<float>()[ptNo] = 0.5 * rectSize.width - (distance - rectSize.width - rectSize.height);
                tY.ptr<float>()[ptNo] = 0.5 * rectSize.height;
                tGX.ptr<float>()[ptNo] = 0;
                tGY.ptr<float>()[ptNo] = -1;
            }
            else {
                tX.ptr<float>()[ptNo] = -0.5 * rectSize.width;
                tY.ptr<float>()[ptNo] = 0.5 * rectSize.height - (distance - rectSize.width * 2 - rectSize.height);
                tGX.ptr<float>()[ptNo] = 1;
                tGY.ptr<float>()[ptNo] = 0;
            }
        }
    }

    if (tVX.cols > 0) {
        outSrcTpl = { tX, tY, tGX, tGY, tVX, tVY };
    }
    else {
        outSrcTpl = { tX, tY, tGX, tGY };
    }

#ifdef _DEBUG
    CheckShapeTemplates(outSrcTpl);
    CheckShapeTemplatesSVG(outSrcTpl);
#endif

    return answer;
}

AnswerType CTemplateShapeBoxType::GetRectSourceTemplateS(const cv::Size2f& inRectSize, const std::vector<int>& inVecLeadNum, const std::vector<double>& inVecLeadLength, const std::vector<double>& inVecLeadWidth, std::vector<double>& inVecCenterX, std::vector<double> inVecCenterY, const float& inCorRad, const std::vector<int>& inVecBoxSide, std::vector<cv::Mat>& outSrcTpl, const float& inSplStep, const int& inPyrLvl) {
    AnswerType answer = IMG_SUCCESS_ANS().SetErrCode();

    cv::Size2f rectSize = inRectSize * powf(0.5, inPyrLvl);
    float corRad = inCorRad * powf(0.5, inPyrLvl);
    std::vector<int> vecLeadNum(inVecLeadNum.begin(), inVecLeadNum.end());
    std::vector<double> vecLeadLength;
    std::vector<double> vecLeadWidth;
    std::vector<double> vecCenterX;
    std::vector<double> vecCenterY;
    for (int i = 0; i < vecLeadNum.size(); ++i) {
        vecLeadLength.push_back(inVecLeadLength[i] * powf(0.5, inPyrLvl));
        vecLeadWidth.push_back(inVecLeadWidth[i] * powf(0.5, inPyrLvl));
        vecCenterX.push_back(inVecCenterX[i] * powf(0.5, inPyrLvl));
        vecCenterY.push_back(inVecCenterY[i] * powf(0.5, inPyrLvl));
    }


    cv::Mat tX, tY, tGX, tGY, tVX, tVY;
    const int leadNum = std::accumulate(vecLeadNum.begin(), vecLeadNum.end(), 0);
    const int ignNum = std::accumulate(inVecBoxSide.begin(), inVecBoxSide.end(), 0);
    if (false) {  // LeadTemplate
        if (vecLeadNum[0] == 1 && vecLeadNum[2] == 1 && vecLeadLength[0] > 0 && vecLeadLength[2] > 0 && vecLeadWidth[0] > 0 && vecLeadWidth[2] > 0) {
            std::vector<cv::Mat> vecLeadTpl0, vecLeadTpl2;
            GetRectLeadTemplate(cv::Size2f(vecLeadWidth[0], vecLeadLength[0]), corRad, vecLeadTpl0, inSplStep);
            GetRectLeadTemplate(cv::Size2f(vecLeadWidth[2], vecLeadLength[2]), corRad, vecLeadTpl2, inSplStep);
            GetRotatedShapeTemplate(vecLeadTpl0, vecLeadTpl0, 90 - mMapIdxToAngle.find(0)->second);
            GetRotatedShapeTemplate(vecLeadTpl2, vecLeadTpl2, 90 - mMapIdxToAngle.find(2)->second);
            GetTranslatedShapeTemplate(vecLeadTpl0, vecLeadTpl0, cv::Point2f(vecCenterX[0], vecCenterY[0]));
            GetTranslatedShapeTemplate(vecLeadTpl2, vecLeadTpl2, cv::Point2f(vecCenterX[2], vecCenterY[2]));
            std::vector<cv::Mat> vecResTpl;
            GetMergedShapeTemplate({ vecLeadTpl0, vecLeadTpl2 }, vecResTpl);
            tX = vecResTpl[0];
            tY = vecResTpl[1];
            tGX = vecResTpl[2];
            tGY = vecResTpl[3];
            if (vecResTpl.size() == 6) {
                tVX = vecResTpl[4];
                tVY = vecResTpl[5];
            }
        }
        else {
            std::vector<cv::Mat> vecLeadTpl1, vecLeadTpl3;
            GetRectLeadTemplate(cv::Size2f(vecLeadWidth[1], vecLeadLength[1]), corRad, vecLeadTpl1, inSplStep);
            GetRectLeadTemplate(cv::Size2f(vecLeadWidth[3], vecLeadLength[3]), corRad, vecLeadTpl3, inSplStep);
            GetRotatedShapeTemplate(vecLeadTpl1, vecLeadTpl1, 90 - mMapIdxToAngle.find(1)->second);
            GetRotatedShapeTemplate(vecLeadTpl3, vecLeadTpl3, 90 - mMapIdxToAngle.find(3)->second);
            GetTranslatedShapeTemplate(vecLeadTpl1, vecLeadTpl1, cv::Point2f(vecCenterX[1], vecCenterY[1]));
            GetTranslatedShapeTemplate(vecLeadTpl3, vecLeadTpl3, cv::Point2f(vecCenterX[3], vecCenterY[3]));
            std::vector<cv::Mat> vecResTpl;
            GetMergedShapeTemplate({ vecLeadTpl1, vecLeadTpl3 }, vecResTpl);
            tX = vecResTpl[0];
            tY = vecResTpl[1];
            tGX = vecResTpl[2];
            tGY = vecResTpl[3];
            if (vecResTpl.size() == 6) {
                tVX = vecResTpl[4];
                tVY = vecResTpl[5];
            }
        }
    }
    else if (false) {
        // 计算周长
        const float perimeter = 2 * CV_PI * corRad + 2 * (rectSize.width + rectSize.height) - 8 * corRad;
        const int ptNum = floorf(perimeter / inSplStep) + 1;
        //if (ptNum < 4)

        // 采样
        tX = cv::Mat::zeros(cv::Size(std::floor(ptNum), 1), CV_32FC1);
        tY = cv::Mat::zeros(cv::Size(std::floor(ptNum), 1), CV_32FC1);
        tGX = cv::Mat::zeros(cv::Size(std::floor(ptNum), 1), CV_32FC1);
        tGY = cv::Mat::zeros(cv::Size(std::floor(ptNum), 1), CV_32FC1);
        for (int ptNo = 0; ptNo < ptNum; ++ptNo) {
            float distance = ptNo * inSplStep;

            if ((distance < 0.5 * CV_PI * corRad) && (inVecBoxSide[0] == 0 && inVecBoxSide[1] == 0)) {
                float angle = distance / (0.5 * CV_PI * corRad) * 90 * CV_PI / 180.; // 弧度制
                tX.ptr<float>()[ptNo] = (-0.5 * rectSize.width + corRad) - (corRad * cosf(angle));
                tY.ptr<float>()[ptNo] = (-0.5 * rectSize.height + corRad) - (corRad * sinf(angle));
                tGX.ptr<float>()[ptNo] = cosf(angle);
                tGY.ptr<float>()[ptNo] = sinf(angle);
            }
            else if ((distance < (0.5 * CV_PI * corRad + rectSize.width - 2 * corRad)) && (inVecBoxSide[0] == 0)) {
                tX.ptr<float>()[ptNo] = (-0.5 * rectSize.width + corRad) + (distance - 0.5 * CV_PI * corRad);
                tY.ptr<float>()[ptNo] = (-0.5 * rectSize.height);
                tGX.ptr<float>()[ptNo] = 0;
                tGY.ptr<float>()[ptNo] = 1;
            }
            else if ((distance < (1.0 * CV_PI * corRad + rectSize.width - 2 * corRad)) && (inVecBoxSide[0] == 0 && inVecBoxSide[3] == 0)) {
                float angle = (distance - (0.5 * CV_PI * corRad + rectSize.width - 2 * corRad)) / (0.5 * CV_PI * corRad) * 90 * CV_PI / 180.; // 弧度制
                tX.ptr<float>()[ptNo] = (0.5 * rectSize.width - corRad) + (corRad * sinf(angle));
                tY.ptr<float>()[ptNo] = (-0.5 * rectSize.height + corRad) - (corRad * cosf(angle));
                tGX.ptr<float>()[ptNo] = -sinf(angle);
                tGY.ptr<float>()[ptNo] = cosf(angle);
            }
            else if ((distance < (1.0 * CV_PI * corRad + rectSize.width + rectSize.height - 4 * corRad)) && (inVecBoxSide[3] == 0)) {
                tX.ptr<float>()[ptNo] = (0.5 * rectSize.width);
                tY.ptr<float>()[ptNo] = (-0.5 * rectSize.height + corRad) + (distance - (1.0 * CV_PI * corRad + rectSize.width - 2 * corRad));
                tGX.ptr<float>()[ptNo] = -1;
                tGY.ptr<float>()[ptNo] = 0;
            }
            else if (((distance < (1.5 * CV_PI * corRad + rectSize.width + rectSize.height - 4 * corRad)) && (inVecBoxSide[2] == 0 && inVecBoxSide[3] == 0)) && (inVecBoxSide[2] == 0 && inVecBoxSide[3] == 0)) {
                float angle = (distance - (1.0 * CV_PI * corRad + rectSize.width + rectSize.height - 4 * corRad)) / (0.5 * CV_PI * corRad) * 90 * CV_PI / 180.; // 弧度制
                tX.ptr<float>()[ptNo] = (0.5 * rectSize.width - corRad) + (corRad * cosf(angle));
                tY.ptr<float>()[ptNo] = (0.5 * rectSize.height - corRad) + (corRad * sinf(angle));
                tGX.ptr<float>()[ptNo] = -cosf(angle);
                tGY.ptr<float>()[ptNo] = -sinf(angle);
            }
            else if ((distance < (1.5 * CV_PI * corRad + 2 * rectSize.width + rectSize.height - 6 * corRad)) && (inVecBoxSide[2] == 0)) {
                tX.ptr<float>()[ptNo] = (0.5 * rectSize.width - corRad) - (distance - (1.5 * CV_PI * corRad + rectSize.width + rectSize.height - 4 * corRad));
                tY.ptr<float>()[ptNo] = (0.5 * rectSize.height);
                tGX.ptr<float>()[ptNo] = 0;
                tGY.ptr<float>()[ptNo] = -1;
            }
            else if ((distance < (2.0 * CV_PI * corRad + 2 * rectSize.width + rectSize.height - 6 * corRad)) && (inVecBoxSide[1] == 0 && inVecBoxSide[2] == 0)) {
                float angle = (distance - (1.5 * CV_PI * corRad + 2 * rectSize.width + rectSize.height - 6 * corRad)) / (0.5 * CV_PI * corRad) * 90 * CV_PI / 180.; // 弧度制
                tX.ptr<float>()[ptNo] = (-0.5 * rectSize.width + corRad) - (corRad * sinf(angle));
                tY.ptr<float>()[ptNo] = (0.5 * rectSize.height - corRad) + (corRad * cosf(angle));
                tGX.ptr<float>()[ptNo] = sinf(angle);
                tGY.ptr<float>()[ptNo] = -cosf(angle);
            }
            else if (inVecBoxSide[1] == 0) {
                tX.ptr<float>()[ptNo] = (-0.5 * rectSize.width);
                tY.ptr<float>()[ptNo] = (0.5 * rectSize.height - corRad) - (distance - (2.0 * CV_PI * corRad + 2 * rectSize.width + rectSize.height - 6 * corRad));
                tGX.ptr<float>()[ptNo] = 1;
                tGY.ptr<float>()[ptNo] = 0;
            }
        }

        outSrcTpl = { tX, tY, tGX, tGY };
    }
    else if (false) {  // 纯矩形
        const float perimeter = 2 * (rectSize.width + rectSize.height);
        const int ptNum = floorf(perimeter / inSplStep) + 1;
        //if (ptNum < 4)

        // 采样
        tX = cv::Mat::zeros(cv::Size(std::floor(ptNum), 1), CV_32FC1);
        tY = cv::Mat::zeros(cv::Size(std::floor(ptNum), 1), CV_32FC1);
        tGX = cv::Mat::zeros(cv::Size(std::floor(ptNum), 1), CV_32FC1);
        tGY = cv::Mat::zeros(cv::Size(std::floor(ptNum), 1), CV_32FC1);
        for (int ptNo = 0; ptNo < ptNum; ++ptNo) {
            float distance = ptNo * inSplStep;

            if (distance < rectSize.width && inVecBoxSide[0] == 0) {
                tX.ptr<float>()[ptNo] = -0.5 * rectSize.width + distance;
                tY.ptr<float>()[ptNo] = -0.5 * rectSize.height;
                tGX.ptr<float>()[ptNo] = 0;
                tGY.ptr<float>()[ptNo] = 1;
            }
            else if (distance < rectSize.width + rectSize.height && inVecBoxSide[1] == 0) {
                tX.ptr<float>()[ptNo] = 0.5 * rectSize.width;
                tY.ptr<float>()[ptNo] = -0.5 * rectSize.height + (distance - rectSize.width);
                tGX.ptr<float>()[ptNo] = -1;
                tGY.ptr<float>()[ptNo] = 0;
            }
            else if (distance < (rectSize.width * 2 + rectSize.height) && inVecBoxSide[2] == 0) {
                tX.ptr<float>()[ptNo] = 0.5 * rectSize.width - (distance - rectSize.width - rectSize.height);
                tY.ptr<float>()[ptNo] = 0.5 * rectSize.height;
                tGX.ptr<float>()[ptNo] = 0;
                tGY.ptr<float>()[ptNo] = -1;
            }
            else {
                tX.ptr<float>()[ptNo] = -0.5 * rectSize.width;
                tY.ptr<float>()[ptNo] = 0.5 * rectSize.height - (distance - rectSize.width * 2 - rectSize.height);
                tGX.ptr<float>()[ptNo] = 1;
                tGY.ptr<float>()[ptNo] = 0;
            }
        }
    }
    else {  // 防偏斜模板
        const int ptNumX = std::max(int((rectSize.width - 2 * corRad) / inSplStep + 1), 2);
        const float splStepX = (rectSize.width - 2 * corRad) / float(ptNumX - 1);
        const int ptNumY = std::max(int((rectSize.height - 2 * corRad) / inSplStep + 1), 2);
        const float splStepY = (rectSize.height - 2 * corRad) / float(ptNumY - 1);

        const float lengthRatio = 0.6 * 0.5;
        const float deltaP = 1;
        const int ptNumAX = std::max(int((rectSize.width - 2 * corRad) * lengthRatio / inSplStep + 1), 2);
        const float splStepAX = (rectSize.width - 2 * corRad) * lengthRatio / float(ptNumAX - 1);
        const int ptNumAY = std::max(int((rectSize.height - 2 * corRad) * lengthRatio / inSplStep + 1), 2);
        const float splStepAY = (rectSize.height - 2 * corRad) * lengthRatio / float(ptNumAY - 1);

        std::vector<int> drawTilt(4, 0);
        if (rectSize.width > rectSize.height) {
            drawTilt[0] = 1;
            drawTilt[2] = 1;
        }
        else {
            drawTilt[1] = 1;
            drawTilt[3] = 1;
        }

        const int totalPtNum =
            (inVecBoxSide[0] == 0 ? ptNumX + 4 * ptNumAX * drawTilt[0] : 0) +
            (inVecBoxSide[2] == 0 ? ptNumX + 4 * ptNumAX * drawTilt[2] : 0) +
            (inVecBoxSide[3] == 0 ? ptNumY + 4 * ptNumAY * drawTilt[3] : 0) +
            (inVecBoxSide[1] == 0 ? ptNumY + 4 * ptNumAY * drawTilt[1] : 0);
        tX = cv::Mat::zeros(cv::Size(totalPtNum, 1), CV_32FC1);
        tY = cv::Mat::zeros(cv::Size(totalPtNum, 1), CV_32FC1);
        tGX = cv::Mat::zeros(cv::Size(totalPtNum, 1), CV_32FC1);
        tGY = cv::Mat::zeros(cv::Size(totalPtNum, 1), CV_32FC1);
        float* pX = tX.ptr<float>();
        float* pY = tY.ptr<float>();
        float* pGX = tGX.ptr<float>();
        float* pGY = tGY.ptr<float>();
        int idx = -1;
        if (inVecBoxSide[0] == 0) {  // 上
            for (int ptNo = 0; ptNo < ptNumX; ++ptNo) {
                ++idx;
                pX[idx] = -0.5 * rectSize.width + corRad + ptNo * splStepX;
                pY[idx] = -0.5 * rectSize.height;
                pGX[idx] = 0;
                pGY[idx] = 1;
            }
            if (drawTilt[0]) {
                for (int ptNo = 0; ptNo < ptNumAX; ++ptNo) {
                    ++idx;
                    pX[idx] = (-0.5 * rectSize.width + corRad) + ptNo * splStepAX;
                    pY[idx] = -0.5 * rectSize.height - deltaP;
                    pGX[idx] = 0;
                    pGY[idx] = 1;
                }
                for (int ptNo = 0; ptNo < ptNumAX; ++ptNo) {
                    ++idx;
                    pX[idx] = (0.5 * rectSize.width - corRad) - ptNo * splStepAX;
                    pY[idx] = -0.5 * rectSize.height - deltaP;
                    pGX[idx] = 0;
                    pGY[idx] = 1;
                }
                for (int ptNo = 0; ptNo < ptNumAX; ++ptNo) {
                    ++idx;
                    pX[idx] = (-0.5 * rectSize.width + corRad) + ptNo * splStepAX;
                    pY[idx] = -0.5 * rectSize.height + deltaP;
                    pGX[idx] = 0;
                    pGY[idx] = 1;
                }
                for (int ptNo = 0; ptNo < ptNumAX; ++ptNo) {
                    ++idx;
                    pX[idx] = (0.5 * rectSize.width - corRad) - ptNo * splStepAX;
                    pY[idx] = -0.5 * rectSize.height + deltaP;
                    pGX[idx] = 0;
                    pGY[idx] = 1;
                }
            }
        }
        if (inVecBoxSide[2] == 0) {  // 下
            for (int ptNo = 0; ptNo < ptNumX; ++ptNo) {
                ++idx;
                pX[idx] = -0.5 * rectSize.width + corRad + ptNo * splStepX;
                pY[idx] = 0.5 * rectSize.height;
                pGX[idx] = 0;
                pGY[idx] = -1;
            }
            if (drawTilt[2]) {
                for (int ptNo = 0; ptNo < ptNumAX; ++ptNo) {
                    ++idx;
                    pX[idx] = (-0.5 * rectSize.width + corRad) + ptNo * splStepAX;
                    pY[idx] = 0.5 * rectSize.height + deltaP;
                    pGX[idx] = 0;
                    pGY[idx] = -1;
                }
                for (int ptNo = 0; ptNo < ptNumAX; ++ptNo) {
                    ++idx;
                    pX[idx] = (0.5 * rectSize.width - corRad) - ptNo * splStepAX;
                    pY[idx] = 0.5 * rectSize.height + deltaP;
                    pGX[idx] = 0;
                    pGY[idx] = -1;
                }
                for (int ptNo = 0; ptNo < ptNumAX; ++ptNo) {
                    ++idx;
                    pX[idx] = (-0.5 * rectSize.width + corRad) + ptNo * splStepAX;
                    pY[idx] = 0.5 * rectSize.height - deltaP;
                    pGX[idx] = 0;
                    pGY[idx] = -1;
                }
                for (int ptNo = 0; ptNo < ptNumAX; ++ptNo) {
                    ++idx;
                    pX[idx] = (0.5 * rectSize.width - corRad) - ptNo * splStepAX;
                    pY[idx] = 0.5 * rectSize.height - deltaP;
                    pGX[idx] = 0;
                    pGY[idx] = -1;
                }
            }
        }
        if (inVecBoxSide[3] == 0) {  // 左
            for (int ptNo = 0; ptNo < ptNumY; ++ptNo) {
                ++idx;
                pX[idx] = -0.5 * rectSize.width;
                pY[idx] = 0.5 * rectSize.height - corRad - ptNo * splStepY;
                pGX[idx] = 1;
                pGY[idx] = 0;
            }
            if (drawTilt[3]) {
                for (int ptNo = 0; ptNo < ptNumAY; ++ptNo) {
                    ++idx;
                    pX[idx] = -0.5 * rectSize.width - deltaP;
                    pY[idx] = (0.5 * rectSize.height - corRad) - ptNo * splStepAY;
                    pGX[idx] = 1;
                    pGY[idx] = 0;
                }
                for (int ptNo = 0; ptNo < ptNumAY; ++ptNo) {
                    ++idx;
                    pX[idx] = -0.5 * rectSize.width - deltaP;
                    pY[idx] = (-0.5 * rectSize.height + corRad) + ptNo * splStepAY;
                    pGX[idx] = 1;
                    pGY[idx] = 0;
                }
                for (int ptNo = 0; ptNo < ptNumAY; ++ptNo) {
                    ++idx;
                    pX[idx] = -0.5 * rectSize.width + deltaP;
                    pY[idx] = (0.5 * rectSize.height - corRad) - ptNo * splStepAY;
                    pGX[idx] = 1;
                    pGY[idx] = 0;
                }
                for (int ptNo = 0; ptNo < ptNumAY; ++ptNo) {
                    ++idx;
                    pX[idx] = -0.5 * rectSize.width + deltaP;
                    pY[idx] = (-0.5 * rectSize.height + corRad) + ptNo * splStepAY;
                    pGX[idx] = 1;
                    pGY[idx] = 0;
                }
            }
        }
        if (inVecBoxSide[1] == 0) {  // 右
            for (int ptNo = 0; ptNo < ptNumY; ++ptNo) {
                ++idx;
                pX[idx] = 0.5 * rectSize.width;
                pY[idx] = 0.5 * rectSize.height - corRad - ptNo * splStepY;
                pGX[idx] = -1;
                pGY[idx] = 0;
            }
            if (drawTilt[1]) {
                for (int ptNo = 0; ptNo < ptNumAY; ++ptNo) {
                    ++idx;
                    pX[idx] = 0.5 * rectSize.width + deltaP;
                    pY[idx] = (0.5 * rectSize.height - corRad) - ptNo * splStepAY;
                    pGX[idx] = -1;
                    pGY[idx] = 0;
                }
                for (int ptNo = 0; ptNo < ptNumAY; ++ptNo) {
                    ++idx;
                    pX[idx] = 0.5 * rectSize.width + deltaP;
                    pY[idx] = (-0.5 * rectSize.height + corRad) + ptNo * splStepAY;
                    pGX[idx] = -1;
                    pGY[idx] = 0;
                }
                for (int ptNo = 0; ptNo < ptNumAY; ++ptNo) {
                    ++idx;
                    pX[idx] = 0.5 * rectSize.width - deltaP;
                    pY[idx] = (0.5 * rectSize.height - corRad) - ptNo * splStepAY;
                    pGX[idx] = -1;
                    pGY[idx] = 0;
                }
                for (int ptNo = 0; ptNo < ptNumAY; ++ptNo) {
                    ++idx;
                    pX[idx] = 0.5 * rectSize.width - deltaP;
                    pY[idx] = (-0.5 * rectSize.height + corRad) + ptNo * splStepAY;
                    pGX[idx] = -1;
                    pGY[idx] = 0;
                }
            }
        }
    }

    // 绘制灰度模板
    if (true) {
        const float stepRatio = 1;
        const float useRatio = 0.9;
        const float lenRatio = 0.3;
        const int ptNumVX = std::max(int(rectSize.width * lenRatio / (inSplStep * stepRatio) + 1), 2);
        const float splStepVX = int(rectSize.width * lenRatio / float(ptNumVX - 1));
        const int ptNumVY = std::max(int(rectSize.height * lenRatio / (inSplStep * stepRatio) + 1), 2);
        const float splStepVY = int(rectSize.height * lenRatio / float(ptNumVY - 1));
        tVX = cv::Mat::zeros(cv::Size((ptNumVX + ptNumVY) * 4, 1), CV_32FC1);
        tVY = cv::Mat::zeros(cv::Size((ptNumVX + ptNumVY) * 4, 1), CV_32FC1);
        float* pVX = tVX.ptr<float>();
        float* pVY = tVY.ptr<float>();
        int iv = -1;
        for (int ptNo = 0; ptNo < ptNumVY; ++ptNo) {  // 左上
            ++iv;
            pVX[iv] = -(rectSize.width * 0.5 - 1);
            pVY[iv] = -(rectSize.height * useRatio * 0.5 - ptNo * splStepVY);
        }
        for (int ptNo = 0; ptNo < ptNumVY; ++ptNo) {  // 左下
            ++iv;
            pVX[iv] = -(rectSize.width * 0.5 - 1);
            pVY[iv] = +(rectSize.height * useRatio * 0.5 - ptNo * splStepVY);
        }
        for (int ptNo = 0; ptNo < ptNumVY; ++ptNo) {  // 右上
            ++iv;
            pVX[iv] = +(rectSize.width * 0.5 - 1);
            pVY[iv] = -(rectSize.height * useRatio * 0.5 - ptNo * splStepVY);
        }
        for (int ptNo = 0; ptNo < ptNumVY; ++ptNo) {  // 右下
            ++iv;
            pVX[iv] = +(rectSize.width * 0.5 - 1);
            pVY[iv] = +(rectSize.height * useRatio * 0.5 - ptNo * splStepVY);
        }
        for (int ptNo = 0; ptNo < ptNumVX; ++ptNo) {  // 上左
            ++iv;
            pVX[iv] = -(rectSize.width * useRatio * 0.5 - ptNo * splStepVX);
            pVY[iv] = -(rectSize.height * 0.5 - 1);
        }
        for (int ptNo = 0; ptNo < ptNumVX; ++ptNo) {  // 上右
            ++iv;
            pVX[iv] = +(rectSize.width * useRatio * 0.5 - ptNo * splStepVX);
            pVY[iv] = -(rectSize.height * 0.5 - 1);
        }
        for (int ptNo = 0; ptNo < ptNumVX; ++ptNo) {  // 下左
            ++iv;
            pVX[iv] = -(rectSize.width * useRatio * 0.5 - ptNo * splStepVX);
            pVY[iv] = +(rectSize.height * 0.5 - 1);
        }
        for (int ptNo = 0; ptNo < ptNumVX; ++ptNo) {  // 下右
            ++iv;
            pVX[iv] = +(rectSize.width * useRatio * 0.5 - ptNo * splStepVX);
            pVY[iv] = +(rectSize.height * 0.5 - 1);
        }
    }

    if (tVX.cols > 0) {
        outSrcTpl = { tX, tY, tGX, tGY, tVX, tVY };
    }
    else {
        outSrcTpl = { tX, tY, tGX, tGY };
    }

#ifdef _DEBUG
    CheckShapeTemplates(outSrcTpl);
    CheckShapeTemplatesSVG(outSrcTpl);
#endif

    return answer;
}

AnswerType CTemplateShapeBoxType::GetRectSizeTolerance(const cv::Size2f& inRectSize, const double& inScaleX, const double& inScaleY, float& outTol) {
    AnswerType answer = IMG_SUCCESS_ANS().SetErrCode();

    float tmpTolerance = std::max(inRectSize.width, inRectSize.height) * 0.06;  // 0.1
    float maxTolerance = 0.2 / std::min(inScaleX, inScaleY);
    outTol = std::min(tmpTolerance, maxTolerance);

    return answer;
}

// 额外匹配
AnswerType GetScalingInvTransMatrix(const cv::Point2d& inSclFac, const cv::Point2d& inRotCtr, const double& inRotAng, const cv::Point2d& inResCtr, cv::Mat& outInvMat) {
    AnswerType answer = IMG_SUCCESS_ANS().SetErrCode();

    const cv::Mat transMat2 = (cv::Mat_<double>(3, 3) << 1, 0, -inResCtr.x, 0, 1, -inResCtr.y, 0, 0, 1);
    const double T = inRotAng / 180. * CV_PI;
    const double sinT = sin(T), cosT = cos(T);
    const cv::Mat rotMat = (cv::Mat_<double>(3, 3) << cosT, -sinT, 0, sinT, cosT, 0, 0, 0, 1);
    const cv::Mat scaleMat = (cv::Mat_<double>(3, 3) << 1. / inSclFac.x, 0, 0, 0, 1. / inSclFac.y, 0, 0, 0, 1);
    const cv::Mat transMat1 = (cv::Mat_<double>(3, 3) << 1, 0, inRotCtr.x, 0, 1, inRotCtr.y, 0, 0, 1);
    const cv::Mat transMat = transMat1 * scaleMat * rotMat * transMat2;
    outInvMat = transMat(cv::Rect(0, 0, 3, 2));

    return answer;
}

// 亚像素
inline AnswerType GetMaxColValX(const cv::Mat& inSrcImg, std::vector<cv::Point2f>& outVecPt, std::vector<float>& outVecVal, const float& inLineUseRatio, const float& inLineRowBegRatio, const float& inLineMagTresh = 0.15) {
    AnswerType answer = IMG_SUCCESS_ANS().SetErrCode();
    if (inSrcImg.type() != CV_32FC1) {
        answer = IMG_PARAM_ERROR_ANS().SetErrCode(IMG_PARAM_ERROR_ANS::ANS2::ImageTypeError);
        return answer;
    }

    std::vector<int> vecTmpNum;
    for (int col = 0; col < inSrcImg.cols; col += 3) {
        int max_len = 0;
        int cur_len = 0;
        for (int row = 0; row < inSrcImg.rows; ++row) {
            float val = inSrcImg.at<float>(row, col);
            if (val > 0.1f) {
                cur_len++;
                if (cur_len > max_len)
                    max_len = cur_len;
            }
            else {
                cur_len = 0;
            }
        }
        vecTmpNum.push_back(max_len);
    }
    std::sort(vecTmpNum.begin(), vecTmpNum.end(), [](int a, int b) {return a > b; });
    if (vecTmpNum.size() <= 0) {
        answer = IMG_PARAM_ERROR_ANS().SetErrCode(IMG_PARAM_ERROR_ANS::ANS2::ConvexHullPointsNumberError);
        return answer;
    }

    const int useNum = 6;
    if (inSrcImg.rows < useNum) {
        answer = IMG_PARAM_ERROR_ANS().SetErrCode(IMG_PARAM_ERROR_ANS::ANS2::ConvexHullPointsNumberError);
        return answer;
    }

    // 添加精确置信度
    const int halfNum = ceil(inSrcImg.rows * 0.5);
    const float begRatio = inLineRowBegRatio;
    const float ratio = (1 - begRatio) / (halfNum - 1);
    std::vector<float> vecRowRatio;
    for (int rowNo = 0; rowNo < halfNum; ++rowNo) {
        vecRowRatio.push_back(begRatio + rowNo * ratio);
    }
    if (inSrcImg.rows % 2 == 0) {
        for (int rowNo = 0; rowNo < halfNum; ++rowNo) {
            vecRowRatio.push_back(vecRowRatio[halfNum - 1 - rowNo]);
        }
    }
    else {
        for (int rowNo = 0; rowNo < (halfNum - 1); ++rowNo) {
            vecRowRatio.push_back(vecRowRatio[halfNum - 2 - rowNo]);
        }
    }

    const float splIns = 1. / 20.;
    std::vector<cv::Point2f> tmpVecPt;
    std::vector<float> tmpVecVal;
    const int imgHalfNum = (inSrcImg.cols - 1) * 0.5;
    int leftNum = 0;
    int rightNum = 0;
    for (int colNo = 0; colNo < inSrcImg.cols; ++colNo) {
        int maxBegRow = 0;
        float maxVal = 0;
        float tmpSum = 0;
        for (int rowNo = 0; rowNo <= inSrcImg.rows - useNum; ++rowNo) {
            if (rowNo == 0) {
                for (int useNo = 0; useNo < useNum; ++useNo) {
                    tmpSum += inSrcImg.ptr<float>(rowNo + useNo)[colNo] * vecRowRatio[rowNo + useNo];
                }
            }
            else {
                tmpSum = tmpSum - inSrcImg.ptr<float>(rowNo - 1)[colNo] * vecRowRatio[rowNo - 1] +
                    inSrcImg.ptr<float>(rowNo + useNum - 1)[colNo] * vecRowRatio[rowNo + useNum - 1];
            }
            if (tmpSum > maxVal) {
                maxVal = tmpSum;
                maxBegRow = rowNo;
            }
        }
        std::vector<cv::Point2f> vecPt;
        for (int i = 0; i < useNum; ++i) {
            vecPt.push_back(cv::Point2f(maxBegRow + i, inSrcImg.ptr<float>(maxBegRow + i)[colNo]));
        }
        if (vecPt.size() <= 1) {
            continue;
        }

        //CSVTool cst("..\\res\\csv\\" + std::to_string(colNo) + ".csv");
        //cst.Export(vecPt);
        const int maxIdx = std::distance(vecPt.begin(), std::max_element(vecPt.begin(), vecPt.end(), [](const cv::Point2f& a, const cv::Point2f& b) {return a.y < b.y; }));
        const int begIdx = maxIdx == 0 ? maxIdx : maxIdx - 1;
        const int endIdx = maxIdx == useNum - 1 ? maxIdx : maxIdx + 1;

        float tmpMaxY = 0;
        float tmpMaxX = vecPt[begIdx].x;

        //// 高斯拟合开始
        //GaussianParams initial_guess(1, vecPt[maxIdx].x, 1);
        //GaussianParams fitted_params = fit_gaussian(vecPt, initial_guess);
        //tmpMaxX = fitted_params.m;
        //tmpMaxY = gaussian_function(tmpMaxX, fitted_params);
        //// 高斯拟合结束

        //// 完整拟合开始
        //FunctionParams initial_guess(1.0, vecPt[maxIdx].x, 1.0, 1.0, 1.0, 1.0);
        //FunctionParams fitted_params = fit_function(vecPt, initial_guess);
        //tmpMaxX = fitted_params.m;
        //tmpMaxY = model_function(tmpMaxX, fitted_params);
        //// 完整拟合结束

        ////// 非对称高斯拟合开始 AG
        ////FunctionParams initial_guess(1, vecPt[maxIdx].x, 1, 1);
        ////FunctionParams fitted_params = fit_function(vecPt, initial_guess);
        ////tmpMaxX = fitted_params.m;
        ////tmpMaxY = model_function(tmpMaxX, fitted_params);
        //Eigen::VectorXd initial_guess(4);
        //initial_guess << 0.9, vecPt[maxIdx].x, 1, 1;
        //Eigen::VectorXd fitted_params = asymmetricGaussianFit(vecPt, initial_guess);
        //tmpMaxX = fitted_params(1);
        ////float p0 = fitted_params(0);
        ////float p1 = fitted_params(1);
        ////float p2 = fitted_params(2);
        ////float p3 = fitted_params(3);
        //tmpMaxY = asymmetricGaussianFunction(tmpMaxX, fitted_params);
        ////// 非对称高斯拟合结束

        ////// 平滑非对称拟合开始 SA
        ////FunctionParams initial_guess(1, vecPt[maxIdx].x, 1, 1);
        ////FunctionParams fitted_params = fit_function(vecPt, initial_guess);
        ////tmpMaxX = fitted_params.m;
        ////tmpMaxY = model_function(tmpMaxX, fitted_params);
        //Eigen::VectorXd initial_guess(4);
        //initial_guess << 0.9, vecPt[maxIdx].x, 0.25, 0.25;
        //Eigen::VectorXd fitted_params = smoothAsymmetricFit(vecPt, initial_guess);
        //tmpMaxX = fitted_params(1);
        ////float p0 = fitted_params(0);
        ////float p1 = fitted_params(1);
        ////float p2 = fitted_params(2);
        ////float p3 = fitted_params(3);
        //tmpMaxY = smoothAsymmetricFunction(tmpMaxX, fitted_params);
        ////// 平滑非对称拟合结束
        
        // 三次样条插值方案 开始
        CubicSpline csCurve;
        csCurve.build(vecPt);
        //for (float iX = vecPt[begIdx].x; iX < vecPt[endIdx].x; iX += splIns) {
        //    const float tmpY = csCurve.interpolate(iX);
        //    //double predicted = model_function(iX, result);
        //    //const float tmpY = predicted;
        //    if (tmpY > tmpMaxY) {
        //        tmpMaxY = tmpY;
        //        tmpMaxX = iX;
        //    }
        //}
        cv::Point2d tmpMaxPt = csCurve.findSplineMaxXY();
        tmpMaxX = tmpMaxPt.x;
        tmpMaxY = tmpMaxPt.y;
        // 三次样条插值方案 结束

        if (tmpMaxY >= inLineMagTresh) {
            tmpVecPt.push_back(cv::Point2f(colNo, inSrcImg.rows - 1 - tmpMaxX));
            tmpVecVal.push_back(tmpMaxY);
            if (colNo < imgHalfNum) {
                ++leftNum;
            }
            else {
                ++rightNum;
            }
        }
    }

    if (tmpVecPt.size() == 0 || leftNum < imgHalfNum * 0.3 || rightNum < imgHalfNum * 0.3) {
        answer = IMG_PARAM_ERROR_ANS().SetErrCode(IMG_PARAM_ERROR_ANS::ANS2::ConvexHullPointsNumberError);
        return answer;
    }

    if (inLineUseRatio < (1 - 2. / tmpVecPt.size())) {
        int accNum = round(tmpVecPt.size() * inLineUseRatio * 0.5);
        outVecPt.clear();
        for (int i = 0; i < accNum; ++i) {
            outVecPt.push_back(tmpVecPt[i]);
            outVecVal.push_back(tmpVecVal[i]);
        }
        for (int i = tmpVecPt.size() - accNum; i < tmpVecPt.size(); ++i) {
            outVecPt.push_back(tmpVecPt[i]);
            outVecVal.push_back(tmpVecVal[i]);
        }
    }
    else {
        outVecPt = tmpVecPt;
        outVecVal = tmpVecVal;
    }

    return answer;
}

struct LineModel {
    float A, B, C;
};

inline LineModel fitLineRANSAC(const std::vector<cv::Point2f>& points,
    int& inrPtNum,
    int maxIter = 1000,
    float distanceThreshold = 1.0f,
    int minInliers = 10)
{
    int bestInlierCount = 0;
    LineModel bestModel{ 0,0,0 };

    if (points.size() < 2) {
        return bestModel;
    }
    std::mt19937 gen(123456);
    const float useRatio = 0.4;
    std::uniform_int_distribution<int> dis(0, int((points.size() - 1) * useRatio));

    for (int iter = 0; iter < maxIter; iter++) {
        int idx1 = dis(gen);
        int idx2 = dis(gen);
        if (idx1 == idx2) continue;

        cv::Point2f p1 = points[idx1];
        cv::Point2f p2 = points[points.size() - 1 - idx2];

        if (cv::norm(p1 - p2) < 1e-6) continue;

        float A = p2.y - p1.y;
        float B = p1.x - p2.x;
        float C = p2.x * p1.y - p1.x * p2.y;

        float normFactor = std::sqrt(A * A + B * B);
        A /= normFactor;
        B /= normFactor;
        C /= normFactor;

        int inlierCount = 0;
        for (auto& pt : points) {
            float dist = std::fabs(A * pt.x + B * pt.y + C);
            if (dist < distanceThreshold) {
                inlierCount++;
            }
        }

        if (inlierCount > bestInlierCount && inlierCount >= minInliers) {
            bestInlierCount = inlierCount;
            bestModel = { A, B, C };
        }
    }

    inrPtNum = bestInlierCount;

    if (bestInlierCount > 0) {
        std::vector<cv::Point2f> inliers;
        for (auto& pt : points) {
            float dist = std::fabs(bestModel.A * pt.x + bestModel.B * pt.y + bestModel.C);
            if (dist < distanceThreshold) {
                inliers.push_back(pt);
            }
        }

        if (inliers.size() >= 2) {
            cv::Vec4f line;
            cv::fitLine(inliers, line, cv::DIST_L2, 0, 1e-2, 1e-2);
            float vx = line[0], vy = line[1];
            float x0 = line[2], y0 = line[3];
            float A = -vy;
            float B = vx;
            float C = -(A * x0 + B * y0);
            float normFactor = std::sqrt(A * A + B * B);
            bestModel = { A / normFactor, B / normFactor, C / normFactor };
        }
    }

    return bestModel;
}

inline AnswerType GetMagBrdPosAndAng(const cv::Mat& inMagImg, const std::vector<cv::Point2f>& inVecPt, const float& inLineDistTresh, float& outMidY, float& outAng, int& outInrPtNum) {
    AnswerType answer = IMG_SUCCESS_ANS().SetErrCode();

    LineModel line = fitLineRANSAC(inVecPt, outInrPtNum, std::min(int(pow(inVecPt.size() * 0.4, 2)), 128), inLineDistTresh, std::max((int)round(inVecPt.size() * 0.1), 1));
    if ((line.A == 0 && line.B == 0 && line.C == 0) || outInrPtNum == 0) {
        answer = IMG_MATCH_FAIL_ANS().SetErrCode(IMG_MATCH_FAIL_ANS::ANS2::LineFitFail);
        return answer;
    }

#ifdef _DEBUG
    cv::Mat showImg;
    cv::cvtColor(inMagImg, showImg, cv::COLOR_GRAY2BGR);
    cv::Point2f pt1(0, 100); // 起点
    cv::Point2f pt2(inMagImg.cols - 1, 300); // 终点
    pt1.y = inMagImg.rows - 1 - (-line.C - line.A * pt1.x) / line.B;
    pt2.y = inMagImg.rows - 1 - (-line.C - line.A * pt2.x) / line.B;
    cv::line(showImg, pt1, pt2, cv::Scalar(0, 1, 0), 1, cv::LINE_AA);

    cv::Point2f svgOffset(0.5, 0.5);
    SVGTool st("..\\res\\0.svg", inMagImg);
    st.drawLine(pt1 + svgOffset, pt2 + svgOffset, 0.3, "blue");
    for (int ptNo1 = 0; ptNo1 < inVecPt.size(); ++ptNo1) {
        st.drawCircle(cv::Point2f(inVecPt[ptNo1].x, inMagImg.rows - 1 - inVecPt[ptNo1].y) + svgOffset, 0.3, "green");
    }
    st.close();
#endif // DEBUG

    // 计算偏移
    const double eps = 1e-12;
    if (std::fabs(line.B) < eps) {
        // B = 0 表示直线是垂直的，y 无法由 x 唯一确定
        answer = IMG_MATCH_FAIL_ANS().SetErrCode(IMG_MATCH_FAIL_ANS::ANS2::LineFitFail);
        return answer;
    }
    outMidY = (-line.A * (inMagImg.cols - 1) * 0.5 - line.C) / line.B;

    // 计算角度
    float tmpAng = std::atan2(line.A, -line.B) * 180. / CV_PI;
    if (abs(tmpAng) > 90) {
        tmpAng > 0 ? tmpAng -= 180 : tmpAng += 180;
    }
    outAng = tmpAng;

    return answer;
}

inline AnswerType GetMagBrdPosAndAngX(const cv::Mat& inMagImg1, const cv::Mat& inMagImg2, const std::vector<cv::Point2f>& inVecPt1, const std::vector<cv::Point2f>& inVecPt2, const float& inLineDistTresh, float& outMidY, float& outAng, int& outInrPtNum) {
    AnswerType answer = IMG_SUCCESS_ANS().SetErrCode();

    std::vector<float> vecPtY1(inMagImg1.cols, -1);
    std::vector<float> vecPtY2(inMagImg1.cols, -1);
    for (int ptNo = 0; ptNo < inVecPt1.size(); ++ptNo) {
        vecPtY1[round(inVecPt1[ptNo].x)] = inVecPt1[ptNo].y;
    }
    for (int ptNo = 0; ptNo < inVecPt2.size(); ++ptNo) {
        vecPtY2[round(inVecPt2[ptNo].x)] = inVecPt2[ptNo].y;
    }
    std::vector<cv::Point2f> vecPt1;
    std::vector<cv::Point2f> vecPt2;
    for (int ptNo = 0; ptNo < inMagImg1.cols; ++ptNo) {
        if (vecPtY1[ptNo] >= 0 && vecPtY2[ptNo] >= 0) {
            vecPt1.push_back(cv::Point2f(ptNo, vecPtY1[ptNo]));
            vecPt2.push_back(cv::Point2f(ptNo, vecPtY2[ptNo]));
        }
    }

    std::vector<cv::Point2f> vecDifPt;
    for (int ptNo = 0; ptNo < vecPt1.size(); ++ptNo) {
        vecDifPt.push_back(cv::Point2f(vecPt1[ptNo].x, vecPt1[ptNo].y - vecPt2[ptNo].y));
    }
    std::vector<cv::Point2f> vecNrPtNum;
    for (int ptNo = 0; ptNo < vecDifPt.size(); ++ptNo) {
        cv::Point2f tmpPt(vecDifPt[ptNo].x, 0);
        for (int tstNo = 0; tstNo < vecDifPt.size(); ++tstNo) {
            if (tstNo == ptNo || abs(vecDifPt[ptNo].y - vecDifPt[tstNo].y) < inLineDistTresh) {
                tmpPt.y += 1;
            }
        }
        vecNrPtNum.push_back(tmpPt);
    }
    if (vecNrPtNum.size() <= 0) {
        answer = IMG_MATCH_FAIL_ANS().SetErrCode(IMG_MATCH_FAIL_ANS::ANS2::CandidatePointsEmpty);
        return answer;
    }

    auto maxItr = std::max_element(vecNrPtNum.begin(), vecNrPtNum.end(), [](const cv::Point2f& inPt1, const cv::Point2f& inPt2) {return inPt1.y < inPt2.y; });
    float maxNum = maxItr->y;

    float minAbsVal = inMagImg1.rows;
    float fitIdx = -1;
    for (int ptNo = 0; ptNo < vecDifPt.size(); ++ptNo) {
        if (abs(vecNrPtNum[ptNo].y - maxNum) < 0.05) {
            if (abs(vecDifPt[ptNo].y) < minAbsVal) {
                minAbsVal = abs(vecDifPt[ptNo].y);
                fitIdx = ptNo;
            }
        }
    }

    outMidY = vecDifPt[fitIdx].y;

    const float fixOffset = outMidY * 0.5;
    std::vector<cv::Point2f> vecTmpPt;
    for (int ptNo = 0; ptNo < vecPt1.size(); ++ptNo) {
        if (abs(vecDifPt[ptNo].y - vecDifPt[fitIdx].y) < inLineDistTresh) {
            vecTmpPt.push_back(cv::Point2f(vecPt1[ptNo].x, vecPt1[ptNo].y - fixOffset));
            vecTmpPt.push_back(cv::Point2f(vecPt2[ptNo].x, vecPt2[ptNo].y + fixOffset));
        }
    }

    LineModel line = fitLineRANSAC(vecTmpPt, outInrPtNum, std::min(int(pow(vecTmpPt.size() * 0.4, 2)), 128), inLineDistTresh, std::max((int)round(vecTmpPt.size() * 0.1), 1));
    if ((line.A == 0 && line.B == 0 && line.C == 0) || outInrPtNum == 0) {
        answer = IMG_MATCH_FAIL_ANS().SetErrCode(IMG_MATCH_FAIL_ANS::ANS2::LineFitFail);
        return answer;
    }

#ifdef _DEBUG
    cv::Mat showImg1, showImg2;
    cv::cvtColor(inMagImg1, showImg1, cv::COLOR_GRAY2BGR);
    cv::cvtColor(inMagImg2, showImg2, cv::COLOR_GRAY2BGR);
    cv::Point2f pt1(0, 100); // 起点
    cv::Point2f pt2(inMagImg1.cols - 1, 300); // 终点
    pt1.y = inMagImg1.rows - 1 - (-line.C - line.A * pt1.x) / line.B;
    pt2.y = inMagImg1.rows - 1 - (-line.C - line.A * pt2.x) / line.B;
    cv::line(showImg1, pt1, pt2, cv::Scalar(0, 1, 0), 1, cv::LINE_AA);
    cv::line(showImg2, pt1, pt2, cv::Scalar(0, 1, 0), 1, cv::LINE_AA);

    cv::Point2f svgOffset(0.5, 0.5);
    SVGTool st1("..\\res\\1.svg", inMagImg1);
    st1.drawLine(pt1 + svgOffset, pt2 + svgOffset, 0.3, "blue");
    for (int ptNo1 = 0; ptNo1 < inVecPt1.size(); ++ptNo1) {
        st1.drawCircle(cv::Point2f(inVecPt1[ptNo1].x, inMagImg1.rows - 1 - inVecPt1[ptNo1].y) + svgOffset, 0.3, "green");
    }
    st1.close();
    SVGTool st2("..\\res\\2.svg", inMagImg2);
    st2.drawLine(pt1 + svgOffset, pt2 + svgOffset, 0.3, "blue");
    for (int ptNo2 = 0; ptNo2 < inVecPt2.size(); ++ptNo2) {
        st2.drawCircle(cv::Point2f(inVecPt2[ptNo2].x, inMagImg2.rows - 1 - inVecPt2[ptNo2].y) + svgOffset, 0.3, "green");
    }
    st2.close();
#endif

    // 计算角度
    float tmpAng = std::atan2(line.A, -line.B) * 180. / CV_PI;
    if (abs(tmpAng) > 90) {
        tmpAng > 0 ? tmpAng -= 180 : tmpAng += 180;
    }
    outAng = tmpAng;

    return answer;
}

template <typename P1, typename P2>
AnswerType GetAffinePoint(const P1& inPt, const cv::Mat& inAffMat, P2& outPt) {
    AnswerType answer = IMG_SUCCESS_ANS().SetErrCode();

    outPt.x = inAffMat.ptr<double>(0)[0] * inPt.x + inAffMat.ptr<double>(0)[1] * inPt.y + inAffMat.ptr<double>(0)[2];
    outPt.y = inAffMat.ptr<double>(1)[0] * inPt.x + inAffMat.ptr<double>(1)[1] * inPt.y + inAffMat.ptr<double>(1)[2];

    return answer;
}

AnswerType CTemplateShapeBoxType::GetPreciseRectPosition(const cv::Point2f& inScale, const cv::Size& inSrcImgSize, const cv::Mat& inCropImg, const cv::Mat& inCropMagImg, const std::vector<cv::Mat>& inCropGradImgs, const cv::Point& inLeftTop, const cv::Size2d& inTotalSize, cv::Point2f& inOffset, float& inAngle, const std::vector<int>& inVecBoxSide, const float& inLineModOffset, const float& inLineModAngle, const float& inLineUseRatio, const float& inLineRowBegRatio, const float& inLineMagTresh, const float& inLineDistTresh) {
    AnswerType answer = IMG_SUCCESS_ANS().SetErrCode();

#if showTimeFlag
    double time_start, tmpTime;
#endif

#if showTimeFlag
    time_start = cv::getTickCount();  // 计时开始
#endif
    // 偏差坐标系转换
    cv::Point2f cmpCtr = mSrcImgCtr + inOffset - cv::Point2f(inLeftTop);

    // 图像转正
    cv::Mat ctrMagImg;
    cm::getCenterScalingAndRotationTransformationImage(inCropMagImg, mScaleFactorInverse, cmpCtr, inAngle, ctrMagImg, inCropMagImg.size(), mCropImgCtr);
    if (std::min(inTotalSize.width, inTotalSize.height) > 60) {  // 大于0603的元件禁用此模块
        answer = IMG_MATCH_FAIL_ANS().SetErrCode(IMG_MATCH_FAIL_ANS::ANS2::BoxPreciseModuleFail);
        return answer;
    }
    cv::Mat invAffMat;
    GetScalingInvTransMatrix(mScaleFactorInverse, cmpCtr, inAngle, mCropImgCtr, invAffMat);
    cv::Point2f affPreOffset = mCropImgCtr;
#if showTimeFlag
    tmpTime = (cv::getTickCount() - time_start) * 1000. / double(cv::getTickFrequency());
    std::cout << " - - 图像转正: " << tmpTime << "ms" << std::endl;  // 计时结束
#endif


#if showTimeFlag
    time_start = cv::getTickCount();  // 计时开始
#endif
    // 四向 截图、边缘点提取、拟合、偏移和角度计算
    const float cropRatio = 0.88;
    const int tbW = inTotalSize.width * cropRatio;
    const int tbH = std::min(inTotalSize.width, inTotalSize.height) * 0.5;
    const cv::Point2f tbCtr((tbW - 1) * 0.5, (tbH - 1) * 0.5);
    const cv::Size tbSize(tbW, tbH);
    const int lrW = inTotalSize.height * cropRatio;
    const int lrH = std::min(inTotalSize.width, inTotalSize.height) * 0.5;
    const cv::Point2f lrCtr((lrW - 1) * 0.5, (lrH - 1) * 0.5);
    const cv::Size lrSize(lrW, lrH);
    std::vector<cv::Mat> vecTransMat = {
        cm::getAffineMatrix(  // 上
            cm::getTranslationTransformationMatrix(-mCropImgCtr + cv::Point2f(0, 0.5 * inTotalSize.height) + tbCtr)
        ),
        cm::getAffineMatrix(  // 下
            cm::getFlipAllTransformationMatrix(tbSize) *
            cm::getTranslationTransformationMatrix(-mCropImgCtr + cv::Point2f(0, -0.5 * inTotalSize.height) + tbCtr)
        ),
        cm::getAffineMatrix(  // 左
            cm::getFlipHorizontalTransformationMatrix(lrSize) *
            cm::getTranspositionTransformationMatrix() *
            cm::getTranslationTransformationMatrix(-mCropImgCtr + cv::Point2f(0.5 * inTotalSize.width, 0) + cv::Point2f(lrCtr.y, lrCtr.x))
        ),
        cm::getAffineMatrix(  // 右
            cm::getFlipVerticalTransformationMatrix(lrSize) *
            cm::getTranspositionTransformationMatrix() *
            cm::getTranslationTransformationMatrix(-mCropImgCtr + cv::Point2f(-0.5 * inTotalSize.width, 0) + cv::Point2f(lrCtr.y, lrCtr.x))
        )
    };
    std::vector<cv::Size> vecImgSize = { tbSize, tbSize, lrSize, lrSize };
    std::vector<cv::Mat> vecAffImg(4);
    std::vector<std::vector<cv::Point2f>> vecPts(4);
    std::vector<std::vector<float>> vecVals(4);
    std::vector<float> vecBrdY(4, 0);
    std::vector<float> vecBrdA(4, 0);
    std::vector<int> vecSkipFlag(4, 0);
    std::vector<int> vecSusFlag(4, 0);
    std::vector<int> vecInrPtNum(4, 0);
    // 根据无效边跳过
    for (int i = 0; i < inVecBoxSide.size(); ++i) {
        if (inVecBoxSide[i] != 0) {
            vecSkipFlag[i] = 1;
        }
    }
#ifdef _DEBUG
    for (int i = 0; i < 4; ++i) {
#else
    concurrency::parallel_for(0, 4, [&](int i) {
#endif
        if (vecSkipFlag[i] == 0) {
            cv::warpAffine(ctrMagImg, vecAffImg[i], vecTransMat[i], vecImgSize[i]);
            answer = GetMaxColValX(vecAffImg[i], vecPts[i], vecVals[i], inLineUseRatio, inLineRowBegRatio, inLineMagTresh);
            if (answer.first != ALGErrCode::IMG_SUCCESS) {
                vecSusFlag[i] = 0;
                answer = IMG_SUCCESS_ANS().SetErrCode();
            }
            else {
                answer = GetMagBrdPosAndAng(vecAffImg[i], vecPts[i], inLineDistTresh, vecBrdY[i], vecBrdA[i], vecInrPtNum[i]);
                if (answer.first == ALGErrCode::IMG_SUCCESS) {
                    if (abs(vecBrdA[i]) < inLineModAngle * 1.2) {
                        vecSusFlag[i] = 1;
                    }
                }
                else {
                    answer = IMG_SUCCESS_ANS().SetErrCode();
                }
            }
        }
# ifdef _DEBUG
        }
# else
    });
# endif
#if showTimeFlag
    tmpTime = (cv::getTickCount() - time_start) * 1000. / double(cv::getTickFrequency());
    std::cout << " - - 四向 截图、边缘点提取、拟合、偏移和角度计算: " << tmpTime << "ms" << std::endl;  // 计时结束
#endif

#if showTimeFlag
    time_start = cv::getTickCount();  // 计时开始
#endif
    // 精确偏移计算
    std::vector<int> vecSusFlagTS(2, 0);
    std::vector<float> vecPreAngle(2, 0);
    std::vector<int> vecPreInrPtNum(2, 0);
    concurrency::parallel_for(0, 2, [&](int i) {
        if (i == 0) {
            if (vecSusFlag[0] == 1 && vecSusFlag[1] == 1) {  // 上下
                const bool arFlag = abs(vecBrdA[0] - vecBrdA[1]) < inLineModAngle;  // 角度检查
                const bool wrFlag = abs(vecBrdY[0] - vecBrdY[1]) < inTotalSize.height * pow(std::sin((vecBrdA[0] + vecBrdA[1]) * 0.5 * CV_PI / 180.), 2) + std::min(inTotalSize.height * 0.2, 0.2 / std::min(inScale.x, inScale.y));  // 宽度检查
                if (arFlag && wrFlag) {
                    float tbBrdY = -1;
                    float tbBrdA = 0;
                    answer = GetMagBrdPosAndAngX(vecAffImg[0], vecAffImg[1], vecPts[0], vecPts[1], inLineDistTresh, tbBrdY, tbBrdA, vecPreInrPtNum[0]);
                    if (answer.first == ALGErrCode::IMG_SUCCESS) {
                        vecSusFlagTS[0] = 1;
                        if (abs((tbBrdY * 0.5) - (vecBrdY[0] - vecBrdY[1]) * 0.5) < inLineModOffset * 0.5 &&
                            abs(tbBrdA - (vecBrdA[0] + vecBrdA[1]) * 0.5) < inLineModAngle * 0.5 &&
                            abs(tbBrdA) < inLineModAngle) {
                            affPreOffset.y -= tbBrdY * 0.5;
                            vecPreAngle[0] = tbBrdA;
                        }
                        else {
                            affPreOffset.y -= (vecBrdY[0] - vecBrdY[1]) * 0.5;
                            vecPreAngle[0] = (vecBrdA[0] + vecBrdA[1]) * 0.5;
                        }
                    }
                    else {
                        answer = IMG_SUCCESS_ANS().SetErrCode();
                    }
                }
            }
        }
        else {
            if (vecSusFlag[2] == 1 && vecSusFlag[3] == 1) {  // 左右
                const bool arFlag = abs(vecBrdA[2] - vecBrdA[3]) < inLineModAngle;  // 角度检查
                const bool wrFlag = abs(vecBrdY[2] - vecBrdY[3]) < inTotalSize.height * pow(std::sin((vecBrdA[2] + vecBrdA[3]) * 0.5 * CV_PI / 180.), 2) + std::min(inTotalSize.height * 0.2, 0.2 / std::min(inScale.x, inScale.y));  // 宽度检查
                if (arFlag && wrFlag) {
                    float lrBrdY = -1;
                    float lrBrdA = 0;
                    answer = GetMagBrdPosAndAngX(vecAffImg[2], vecAffImg[3], vecPts[2], vecPts[3], inLineDistTresh, lrBrdY, lrBrdA, vecPreInrPtNum[1]);
                    if (answer.first == ALGErrCode::IMG_SUCCESS) {
                        vecSusFlagTS[1] = 1;
                        if (abs((lrBrdY * 0.5) - (vecBrdY[2] - vecBrdY[3]) * 0.5) < inLineModOffset * 0.5 &&
                            abs(lrBrdA - (vecBrdA[2] + vecBrdA[3]) * 0.5) < inLineModAngle * 0.5 &&
                            abs(lrBrdA) < inLineModAngle) {
                            affPreOffset.x -= lrBrdY * 0.5;
                            vecPreAngle[1] = lrBrdA;
                        }
                        else {
                            affPreOffset.x -= (vecBrdY[2] - vecBrdY[3]) * 0.5;
                            vecPreAngle[1] = (vecBrdA[2] + vecBrdA[3]) * 0.5;
                        }
                    }
                    else {
                        answer = IMG_SUCCESS_ANS().SetErrCode();
                    }
                }
            }
        }
        });


#if showTimeFlag
    tmpTime = (cv::getTickCount() - time_start) * 1000. / double(cv::getTickFrequency());
    std::cout << " - - 精确偏移计算: " << tmpTime << "ms" << std::endl;  // 计时结束
#endif


    // 偏移坐标系转换
    cv::Point2f prePos(0, 0);
    GetAffinePoint(affPreOffset, invAffMat, prePos);
    cv::Point2f outPreOffset = prePos - cmpCtr;

    // 角度计算
    float outPreAngle = 0;
    float angRng = 2 * atan2(1., std::min(inTotalSize.width, inTotalSize.height)) * 180. / CV_PI;
    float acpAngRng = std::min(inLineModAngle * (float)0.5, abs(angRng));
    if (vecSusFlagTS[0] + vecSusFlagTS[1] == 2) {
        if (abs(vecPreAngle[0] - vecPreAngle[1]) < acpAngRng) {
            outPreAngle = -(vecPreAngle[0] * vecPreInrPtNum[0] + vecPreAngle[1] * vecPreInrPtNum[1]) / (vecPreInrPtNum[0] + vecPreInrPtNum[1]);
        }
        else {
            if (vecPreInrPtNum[0] / sqrt(inTotalSize.width) > vecPreInrPtNum[1] / sqrt(inTotalSize.height)) {
                if (abs(vecBrdA[2] - vecPreAngle[0]) < acpAngRng) {
                    outPreAngle = -(vecPreAngle[0] * vecPreInrPtNum[0] / sqrt(inTotalSize.width) + vecBrdA[2] * vecInrPtNum[2] / sqrt(inTotalSize.height)) / (vecPreInrPtNum[0] / sqrt(inTotalSize.width) + vecInrPtNum[2] / sqrt(inTotalSize.height));
                }
                else if (abs(vecBrdA[3] - vecPreAngle[0]) < acpAngRng) {
                    outPreAngle = -(vecPreAngle[0] * vecPreInrPtNum[0] / sqrt(inTotalSize.width) + vecBrdA[3] * vecInrPtNum[3] / sqrt(inTotalSize.height)) / (vecPreInrPtNum[0] / sqrt(inTotalSize.width) + vecInrPtNum[3] / sqrt(inTotalSize.height));
                }
                else {
                    outPreAngle = -vecPreAngle[0];
                }
            }
            else {
                if (abs(vecBrdA[0] - vecPreAngle[1]) < acpAngRng) {
                    outPreAngle = -(vecPreAngle[1] * vecPreInrPtNum[1] / sqrt(inTotalSize.height) + vecBrdA[0] * vecInrPtNum[0] / sqrt(inTotalSize.width)) / (vecPreInrPtNum[1] / sqrt(inTotalSize.height) + vecInrPtNum[0] / sqrt(inTotalSize.width));
                }
                else if (abs(vecBrdA[1] - vecPreAngle[1]) < acpAngRng) {
                    outPreAngle = -(vecPreAngle[1] * vecPreInrPtNum[1] / sqrt(inTotalSize.height) + vecBrdA[1] * vecInrPtNum[1] / sqrt(inTotalSize.width)) / (vecPreInrPtNum[1] / sqrt(inTotalSize.height) + vecInrPtNum[1] / sqrt(inTotalSize.width));
                }
                else {
                    outPreAngle = -vecPreAngle[1];
                }
            }
        }
    }
    else if (vecSusFlagTS[0] == 1) {
        if (inTotalSize.width > inTotalSize.height) {
            if (abs(vecBrdA[2] - vecPreAngle[0]) < acpAngRng && vecSusFlag[2] == 1) {
                outPreAngle = -(vecPreAngle[0] * vecPreInrPtNum[0] / sqrt(inTotalSize.width) + vecBrdA[2] * vecInrPtNum[2] / sqrt(inTotalSize.height)) / (vecPreInrPtNum[0] / sqrt(inTotalSize.width) + vecInrPtNum[2] / sqrt(inTotalSize.height));
            }
            else if (abs(vecBrdA[3] - vecPreAngle[0]) < acpAngRng && vecSusFlag[3] == 1) {
                outPreAngle = -(vecPreAngle[0] * vecPreInrPtNum[0] / sqrt(inTotalSize.width) + vecBrdA[3] * vecInrPtNum[3] / sqrt(inTotalSize.height)) / (vecPreInrPtNum[0] / sqrt(inTotalSize.width) + vecInrPtNum[3] / sqrt(inTotalSize.height));
            }
            else {
                outPreAngle = -vecPreAngle[0];
            }
        }
        else {
            if (abs(vecBrdA[2] - vecPreAngle[0]) < acpAngRng && vecSusFlag[2] == 1) {
                outPreAngle = -(vecPreAngle[0] * vecPreInrPtNum[0] / sqrt(inTotalSize.width) + vecBrdA[2] * vecInrPtNum[2] / sqrt(inTotalSize.height)) / (vecPreInrPtNum[0] / sqrt(inTotalSize.width) + vecInrPtNum[2] / sqrt(inTotalSize.height));
            }
            else if (abs(vecBrdA[3] - vecPreAngle[0]) < acpAngRng && vecSusFlag[3] == 1) {
                outPreAngle = -(vecPreAngle[0] * vecPreInrPtNum[0] / sqrt(inTotalSize.width) + vecBrdA[3] * vecInrPtNum[3] / sqrt(inTotalSize.height)) / (vecPreInrPtNum[0] / sqrt(inTotalSize.width) + vecInrPtNum[3] / sqrt(inTotalSize.height));
            }
        }
    }
    else if (vecSusFlagTS[1] == 1) {
        if (inTotalSize.height > inTotalSize.width) {
            if (abs(vecBrdA[0] - vecPreAngle[1]) < acpAngRng && vecSusFlag[0] == 1) {
                outPreAngle = -(vecPreAngle[1] * vecPreInrPtNum[1] / sqrt(inTotalSize.height) + vecBrdA[0] * vecInrPtNum[0] / sqrt(inTotalSize.width)) / (vecPreInrPtNum[1] / sqrt(inTotalSize.height) + vecInrPtNum[0] / sqrt(inTotalSize.width));
            }
            else if (abs(vecBrdA[1] - vecPreAngle[1]) < acpAngRng && vecSusFlag[1] == 1) {
                outPreAngle = -(vecPreAngle[1] * vecPreInrPtNum[1] / sqrt(inTotalSize.height) + vecBrdA[1] * vecInrPtNum[1] / sqrt(inTotalSize.width)) / (vecPreInrPtNum[1] / sqrt(inTotalSize.height) + vecInrPtNum[1] / sqrt(inTotalSize.width));
            }
            else {
                outPreAngle = -vecPreAngle[1];
            }
        }
        else {
            if (abs(vecBrdA[0] - vecPreAngle[1]) < acpAngRng && vecSusFlag[0] == 1) {
                outPreAngle = -(vecPreAngle[1] * vecPreInrPtNum[1] / sqrt(inTotalSize.height) + vecBrdA[0] * vecInrPtNum[0] / sqrt(inTotalSize.width)) / (vecPreInrPtNum[1] / sqrt(inTotalSize.height) + vecInrPtNum[0] / sqrt(inTotalSize.width));
            }
            else if (abs(vecBrdA[1] - vecPreAngle[1]) < acpAngRng && vecSusFlag[1] == 1) {
                outPreAngle = -(vecPreAngle[1] * vecPreInrPtNum[1] / sqrt(inTotalSize.height) + vecBrdA[1] * vecInrPtNum[1] / sqrt(inTotalSize.width)) / (vecPreInrPtNum[1] / sqrt(inTotalSize.height) + vecInrPtNum[1] / sqrt(inTotalSize.width));
            }
        }
    }

    // 结果输出
    if (answer.first == ALGErrCode::IMG_SUCCESS) {
        inOffset += outPreOffset;
        inAngle += outPreAngle;
    }

    return answer;
}

// 结果检查
inline AnswerType GetSideCropImg(const cv::Mat& inImg, cv::Mat& outImg, const float& inCropRatio = 0.1) {
    AnswerType answer = IMG_SUCCESS_ANS().SetErrCode();

    const int lr = inImg.cols * inCropRatio;
    const int tb = inImg.rows * inCropRatio;

    if (inImg.cols - 2 * lr <= 0 || inImg.rows - 2 * tb <= 0) {
        outImg = inImg.clone();
    }
    else {
        cv::Rect crop(lr, tb, inImg.cols - 2 * lr, inImg.rows - 2 * tb);
        outImg = inImg(crop).clone();
    }

    return answer;
}

inline AnswerType GetSplitImg(const cv::Mat& inImg, cv::Mat& outImg0, cv::Mat& outImg1, const bool& inHorFlg = true) {
    AnswerType answer = IMG_SUCCESS_ANS().SetErrCode();

    if (inHorFlg) {
        const int halfWidth = inImg.cols * 0.5;
        const cv::Rect rectL(0, 0, halfWidth, inImg.rows);
        const cv::Rect rectR(inImg.cols - halfWidth, 0, halfWidth, inImg.rows);
        outImg0 = inImg(rectL).clone();
        outImg1 = inImg(rectR).clone();
    }
    else {
        const int halfHeight = inImg.rows * 0.5;
        const cv::Rect rectL(0, 0, inImg.cols, halfHeight);
        const cv::Rect rectR(0, inImg.rows - halfHeight, inImg.cols, halfHeight);
        outImg0 = inImg(rectL).clone();
        outImg1 = inImg(rectR).clone();
    }

    return answer;
}
