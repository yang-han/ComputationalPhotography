#pragma once

#include <iostream>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/opencv.hpp"
#include "Dinic.h"
#include "GaussMixtureModel.h"

using namespace cv;

class GrabCutImplementation {
public:
    double calculateBeta(const Mat &image);

    void calMaskByBox(Mat payloadMask, Rect box);

    void doBorderMatting();

    void grabCut(cv::InputArray imageInputArray, cv::InputOutputArray maskIOArray, int iterCount);

private:
    Mat payloadImage;
    Mat payloadMask;

    void calculateNWeights(const Mat image, double beta, Mat left, Mat upLeft, Mat up, Mat upRight);

    void constructNetwork(const Mat image, Mat mask, Mat left, Mat upLeft, Mat up, Mat upRight,
                          GaussMixtureModel &backgroundGmm,
                          GaussMixtureModel &foregroundGMM, Dinic *network);
};

