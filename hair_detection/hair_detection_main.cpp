#include "hair_detection_main.h"

bool hairDetection(cv::Mat& src, cv::Mat& dst, bool isGPU) {
    if (!src.data) {
        std::cout << "Error: the image wasn't correctly loaded." << std::endl;
        return false;
    }

    if (src.type() != CV_8UC3) {
        std::cout << "input image must be CV_8UC3! " << std::endl;
        return false;
    }

    HairDetectionInfo para;
    
#if TIMER
    auto t1 = std::chrono::system_clock::now();
#endif

    cv::Mat mask(cv::Size(src.cols, src.rows), CV_8U, cv::Scalar(0));
    if (isGPU) {
        getHairMaskGPU(src, mask, para);
    }
    else {
        getHairMaskCPU(src, mask, para);
    }

#if TIMER
    auto t4 = std::chrono::system_clock::now();
#endif

    cv::Mat glcm(cv::Size(DYNAMICRANGE, DYNAMICRANGE), CV_32F, cv::Scalar(0));
    getGrayLevelCoOccurrenceMatrix(mask, glcm);

#if TIMER
    auto t5 = std::chrono::system_clock::now();
#endif

    int threshold = isGPU? entropyThesholdingGPU(glcm) : entropyThesholding(glcm);
    cv::threshold(mask, mask, threshold, DYNAMICRANGE - 1, 0);
    glcm.release();

#if TIMER
    auto t6 = std::chrono::system_clock::now();
#endif

    cleanIsolatedComponent(mask, para);

#if TIMER
    auto t7 = std::chrono::system_clock::now();
#endif

    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5), cv::Point(-1, -1));
    cv::morphologyEx(mask, mask, cv::MORPH_DILATE, kernel, cv::Point(-1, -1), 1);
    //inpaintHair(src, dst, mask, para);

    dst = mask;

#if TIMER
    printTime(t1, t4, "main -- get hair mask");
    printTime(t4, t5, "main -- glcm_cal");
    printTime(t5, t6, "main -- entropyThesholding");
    printTime(t6, t7, "main -- cleanIsolatedComponent");
#endif
    return true;
}