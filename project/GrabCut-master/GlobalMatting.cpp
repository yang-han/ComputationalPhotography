//
// Created by zouyao on 6/17/17.
//

#include <iostream>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <chrono>
#include "GlobalMatting.h"
std::vector<cv::Point> GlobalMatting::findBoundaryPixels(const cv::Mat &trimap, int a, int b)
{
    std::vector<cv::Point> result;
    int k = 3;

    for (int x = k; x < trimap.cols - k; ++x)
        for (int y = k; y < trimap.rows - k; ++y) {
            if (trimap.at<uchar>(y, x) == a) {
                bool found = false;
                for (int i = -k; i < k && !found; i++)
                {
                    for (int j = -k; j < k && !found; j++)
                    {
                        if (trimap.at<uchar>(y+i, x+j) == b)
                        {
                            result.push_back(cv::Point(x, y));
                            found = true;
                        }
                    }

                }
            }
        }

    return result;
}

double GlobalMatting::calculateAlpha(const cv::Vec3f &imageColor, const cv::Vec3f &fgdColor,
                                     const cv::Vec3f &bgdColor)
{
    cv::Vec3f imageBgdDiff = imageColor - bgdColor;
    cv::Vec3f fgdBgdDiff = fgdColor - bgdColor;
    double alpha = double(imageBgdDiff.dot(fgdBgdDiff)) / (fgdBgdDiff.dot(fgdBgdDiff));
    return std::min(std::max(alpha, 0.0), 1.0);
}

double GlobalMatting::getColorCost(const cv::Vec3f &imageColor, const cv::Vec3f &fgdColor,
                                   const cv::Vec3f &bgdColor, double alpha)
{
    cv::Vec3f estimatedImageColor = alpha * fgdColor + (1 - alpha) * bgdColor;
    return cv::norm(imageColor - estimatedImageColor);
}

double GlobalMatting::getDistCost(const cv::Point &imagePoint, const cv::Point &boundaryPoint,
                                  double minDist)
{
    double dist = cv::norm(imagePoint - boundaryPoint);
    return dist / minDist;
}

double GlobalMatting::getNearestDistance(const std::vector<cv::Point> &boundary, const cv::Point &p)
{
    auto it = std::min_element(boundary.begin(), boundary.end(),
                               [p](cv::Point left, cv::Point right)
                               {
                                   return cv::norm(p - left) < cv::norm(p - right);
                               });
    cv::Point nearestPoint = *it;
    return cv::norm(nearestPoint - p);
}

std::pair<double, double>
GlobalMatting::getCostAndAlpha(const cv::Point &imagePoint, const cv::Point &fgdPoint,
                               const cv::Point &bgdPoint, const cv::Mat &image,
                               double fgdDist, double bgdDist)
{
    double fgdDistCost = getDistCost(imagePoint, fgdPoint, fgdDist);
    double bgdDistCost = getDistCost(imagePoint, bgdPoint, bgdDist);
    cv::Vec3b imageColor = image.at<cv::Vec3b>(imagePoint);
    cv::Vec3b fgdColor = image.at<cv::Vec3b>(fgdPoint);
    cv::Vec3b bgdColor = image.at<cv::Vec3b>(bgdPoint);
    double alpha = calculateAlpha(imageColor, fgdColor, bgdColor);
    double colorCost = getColorCost(imageColor, fgdColor, bgdColor, alpha);
    double totalCost = fgdDistCost + bgdDistCost + 3 * colorCost;
    return std::make_pair(totalCost, alpha);
}

auto GlobalMatting::globalSampleMatch(const cv::Mat &image,
                                      const cv::Mat &trimap,
                                      const std::vector<cv::Point> &foregroundBoundary,
                                      const std::vector<cv::Point> &backgroundBoundary)
-> std::vector<std::vector<SampleInfo>>
{
    std::vector<std::vector<SampleInfo>> samples;
    samples.resize(size_t(image.rows), std::vector<SampleInfo>(size_t(image.cols)));
    std::vector<cv::Point> imagePointCollection;
    //initialization
    for (int y = 0; y < image.rows; y++) {
        for (int x = 0; x < image.cols; x++) {
            if (trimap.at<uchar>(y, x) == 128) {
                SampleInfo &sInfo = samples[y][x];
                sInfo.fgdIndex = int(rand() % foregroundBoundary.size());
                sInfo.bgdIndex = int(rand() % backgroundBoundary.size());
                cv::Point p(x, y);
                sInfo.fgdDist = getNearestDistance(foregroundBoundary, p);
                sInfo.bgdDist = getNearestDistance(backgroundBoundary, p);
                imagePointCollection.push_back(p);
                sInfo.cost = std::numeric_limits<double>::max();
            }
        }
    }
    //perform global sampling
    for (int iter = 0; iter < 10; iter++) {
        std::random_shuffle(imagePointCollection.begin(), imagePointCollection.end());
        int windowWidth = (int) std::max(foregroundBoundary.size(), backgroundBoundary.size());
        for (const auto &imagePoint : imagePointCollection) {
            //propagation
            cv::Vec3b imageColor = image.at<cv::Vec3b>(imagePoint);
            SampleInfo &s = samples[imagePoint.y][imagePoint.x];

            for (int y2 = imagePoint.y - 1; y2 <= imagePoint.y + 1; y2++)
                for (int x2 = imagePoint.x - 1; x2 <= imagePoint.x + 1; x2++) {
                    if (y2 >= 0 && y2 < image.rows && x2 >= 0 && x2 < image.cols
                        && trimap.at<uchar>(y2, x2) == 128) {
                        const SampleInfo &neighborSample = samples[y2][x2];
                        int fgdIndex = neighborSample.fgdIndex;
                        int bgdIndex = neighborSample.bgdIndex;
                        cv::Point fgdPoint = foregroundBoundary[fgdIndex];
                        cv::Point bgdPoint = backgroundBoundary[bgdIndex];
                        double cost;
                        double alpha;
                        std::tie(cost, alpha) = getCostAndAlpha(imagePoint, fgdPoint, bgdPoint,
                                                                image, s.fgdDist, s.bgdDist);
                        if (cost < s.cost) {
                            s.fgdIndex = fgdIndex;
                            s.bgdIndex = bgdIndex;
                            s.cost = cost;
                            s.alpha = alpha;
                        }
                    }
                }
            //random search
            for (int k = windowWidth; k >= 1; k /= 2) {
                int fgdDelta = int(k * ((double)rand() / RAND_MAX * 2 -1));
                int bgdDelta = int(k * ((double)rand() / RAND_MAX * 2 -1));
                int fgdIndex = s.fgdIndex + fgdDelta;
                int bgdIndex = s.fgdIndex + bgdDelta;
                if (fgdIndex >= 0 && fgdIndex < foregroundBoundary.size()
                    && bgdIndex >= 0 && bgdIndex < backgroundBoundary.size()) {
                    cv::Point fgdPoint = foregroundBoundary[fgdIndex];
                    cv::Point bgdPoint = backgroundBoundary[bgdIndex];
                    double cost;
                    double alpha;
                    std::tie(cost, alpha) = getCostAndAlpha(imagePoint, fgdPoint, bgdPoint,
                                                            image, s.fgdDist, s.bgdDist);
                    if (cost < s.cost) {
                        s.fgdIndex = fgdIndex;
                        s.bgdIndex = bgdIndex;
                        s.cost = cost;
                        s.alpha = alpha;
                    }
                }
            }
        }
    }
    return samples;
}
void GlobalMatting::globalMatting(const cv::Mat &image, const cv::Mat &trimap,
                                  cv::Mat &foreground, cv::Mat &alpha)
{
    std::cout << "computing global matting" << std::endl;
    std::chrono::time_point<std::chrono::system_clock> start, end;
    start = std::chrono::system_clock::now();

    srand(unsigned(time(nullptr)));
    std::vector<cv::Point> foregroundBoundary = findBoundaryPixels(trimap, 255, 128);
    std::vector<cv::Point> backgroundBoundary = findBoundaryPixels(trimap, 0, 128);
    int n = (int)(foregroundBoundary.size() + backgroundBoundary.size());
    for (int i = 0; i < n; ++i)
    {
        int x = rand() % trimap.cols;
        int y = rand() % trimap.rows;

        if (trimap.at<uchar>(y, x) == 0)
            backgroundBoundary.push_back(cv::Point(x, y));
        else if (trimap.at<uchar>(y, x) == 255)
            foregroundBoundary.push_back(cv::Point(x, y));
    }
    auto intensityComp = [&image](cv::Point left, cv::Point right)
    {
        return cv::norm(image.at<cv::Vec3b>(left)) < cv::norm(image.at<cv::Vec3b>(right));
    };
    std::sort(foregroundBoundary.begin(), foregroundBoundary.end(), intensityComp);
    std::sort(backgroundBoundary.begin(), backgroundBoundary.end(), intensityComp);

    auto samples = globalSampleMatch(image, trimap, foregroundBoundary, backgroundBoundary);

    foreground.create(image.size(), CV_8UC3);
    alpha.create(image.size(), CV_32FC1);
    for (int y = 0; y < alpha.rows; ++y)
        for (int x = 0; x < alpha.cols; ++x) {
            switch (trimap.at<uchar>(y, x)) {
                case 0:alpha.at<float>(y, x) = 0;
                    foreground.at<cv::Vec3b>(y, x) = 0;
                    break;
                case 128: {
                    alpha.at<float>(y, x) = (float)(samples[y][x].alpha);
                        cv::Point p = foregroundBoundary[samples[y][x].fgdIndex];
                        foreground.at<cv::Vec3b>(y, x) = image.at<cv::Vec3b>(p);
                    break;
                }
                case 255:alpha.at<float>(y, x) = 1;
                    foreground.at<cv::Vec3b>(y, x) = image.at<cv::Vec3b>(y, x);
                    break;
            }
        }

    end = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = end-start;
    std::cout << "elapsed time: " << elapsed_seconds.count() << "s\n";
}
