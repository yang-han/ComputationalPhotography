//
// Created by zouyao on 6/16/17.
//
#include <chrono>
#include "TrimapGenerator.h"
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <queue>

std::vector<std::vector<cv::Point>>
TrimapGenerator::parameterizeContour(const cv::Mat &mask)
{
    cv::Mat edge(mask.rows, mask.cols, CV_8UC1);
    cv::Canny(mask, edge, 3, 9);
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(edge, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
    return contours;
}
cv::Mat TrimapGenerator::getTrimap(const cv::Mat &image, const cv::Mat &mask)
{
    std::cout << "get trimap" << std::endl;
    cv::Mat binMask(mask.size(), CV_8U);
    binMask = mask & 1;
    binMask.convertTo(binMask, CV_8U, 255);
    std::chrono::time_point<std::chrono::system_clock> start, end;
    start = std::chrono::system_clock::now();

    auto contours = parameterizeContour(binMask);
    cv::Mat trimap = constructTrimap(contours, mask);

    end = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = end-start;

    std::cout << "elapsed time: " << elapsed_seconds.count() << "s\n";
    return trimap;
}

cv::Mat TrimapGenerator::constructTrimap(const std::vector<std::vector<cv::Point>> &contours, cv::Mat mask)
{
    cv::Mat trimap(mask.size(), CV_8U);
    cv::Point p;
    for (p.y = 0; p.y < mask.rows; p.y++)
        for (p.x = 0; p.x < mask.cols; p.x++) {
            if (mask.at<uchar>(p) == cv::GC_BGD || mask.at<uchar>(p) == cv::GC_PR_BGD) {
                trimap.at<uchar>(p) = BGD;
            }
            else {
                trimap.at<uchar>(p) = FGD;
            }
        }
    cv::Mat distance(mask.size(), CV_8U, cv::Scalar(255));
    std::queue<cv::Point> q;
    for (int i = 0; i < contours.size(); i++)
    {
        for (int j = 0; j < contours[i].size(); j++)
        {
            cv::Point currentPoint = contours[i][j];
            distance.at<uchar>(currentPoint) = 0;
            q.push(currentPoint);
            trimap.at<uchar>(currentPoint) = UNKNOWN;
        }
    }
    while(!q.empty())
    {
        cv::Point currentPoint = q.front();
        q.pop();
        uchar currentDist = distance.at<uchar>(currentPoint);
        if (currentDist < UNKNOWN_WIDTH) {
            for (int i = -1; i <= 1; i++) {
                for (int j = -1; j <= 1; j++) {
                    if ((i == 0) ^ (j == 0)) {
                        cv::Point neighborPoint = currentPoint + cv::Point(i, j);
                        if (neighborPoint.x >= 0 && neighborPoint.x < mask.cols
                            && neighborPoint.y >= 0 && neighborPoint.y < mask.rows) {
                            uchar newDist = currentDist + 1;
                            if (distance.at<uchar>(neighborPoint) > newDist){
                                if (distance.at<uchar>(neighborPoint) != 255){
                                    std::cout << "distance at " << neighborPoint << " updated" << std::endl;
                                }
                                distance.at<uchar>(neighborPoint) = newDist;
                                q.push(neighborPoint);
                                trimap.at<uchar>(neighborPoint) = UNKNOWN;
                            }
                        }
                    }
                }
            }
        }

    }
    return trimap;
}
