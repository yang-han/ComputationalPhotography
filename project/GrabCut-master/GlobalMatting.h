//
// Created by zouyao on 6/17/17.
//

#ifndef GRABCUT_GLOBALMATTING_H
#define GRABCUT_GLOBALMATTING_H

#include <opencv2/core/core.hpp>
class GlobalMatting
{
public:
    void globalMatting(const cv::Mat &image, const cv::Mat &trimap,
                       cv::Mat &foreground, cv::Mat &alpha);
private:
    struct SampleInfo
    {
        int fgdIndex;
        int bgdIndex;
        double fgdDist;
        double bgdDist;
        double cost;
        double alpha;
    };
    std::vector<cv::Point> findBoundaryPixels(const cv::Mat &trimap, int a, int b);
    double calculateAlpha(const cv::Vec3f &imageColor, const cv::Vec3f &fgdColor,
                          const cv::Vec3f &bgdColor);
    double getColorCost(const cv::Vec3f &imageColor, const cv::Vec3f &fgdColor,
                        const cv::Vec3f &bgdColor, double alpha);
    double getDistCost(const cv::Point &imagePoint, const cv::Point &boundaryPoint, double minDist);
    double getNearestDistance(const std::vector<cv::Point> &boundary, const cv::Point &p);
    std::pair<double, double>
    getCostAndAlpha(const cv::Point &imagePoint, const cv::Point &fgdPoint,
                    const cv::Point &bgdPoint, const cv::Mat &image,
                    double fgdDist, double bgdDist);
    std::vector<std::vector<SampleInfo>>
    globalSampleMatch(const cv::Mat &image, const cv::Mat &trimap,
                      const std::vector<cv::Point> &foregroundBoundary,
                      const std::vector<cv::Point> &backgroundBoundary);
};

#endif //GRABCUT_GLOBALMATTING_H
