//
// Created by zouyao on 6/16/17.
//

#ifndef GRABCUT_TRIMAPGENERATOR_H
#define GRABCUT_TRIMAPGENERATOR_H
#include <opencv2/core/core.hpp>
#include <map>

class TrimapGenerator
{
public:
    cv::Mat getTrimap(const cv::Mat &image, const cv::Mat &mask);
private:
    const static int UNKNOWN_WIDTH = 2;
    enum {BGD=0, UNKNOWN=128, FGD=255};
    std::vector<std::vector<cv::Point>> parameterizeContour(const cv::Mat &mask);
    cv::Mat constructTrimap(const std::vector<std::vector<cv::Point>> &contours, cv::Mat mask);
};


#endif //GRABCUT_TRIMAPGENERATOR_H
