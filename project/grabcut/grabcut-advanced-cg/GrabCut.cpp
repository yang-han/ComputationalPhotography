#include "GrabCut.h"
#include "GaussMixtureModel.h"
#include "BorderMatting.h"
#include "Dinic.h"

using namespace cv;
using namespace std;
const double gamma_ = 50;
const double lambda = gamma_ * 9;
BorderMatting borderMatting;

void GrabCutImplementation::calMaskByBox(Mat payloadMask, Rect box) {
    payloadMask = Mat::zeros(payloadImage.size(), CV_8U);
    for (int row = 0; row < payloadMask.rows; row++)
        for (int col = 0; col < payloadMask.cols; col++) {
            if (col < box.tl().x || col > box.tl().x + box.width || row < box.tl().y || row > box.tl().y + box.height)
                payloadMask.at<uchar>(row, col) = GC_BGD;
            else
                payloadMask.at<uchar>(row, col) = GC_PR_FGD;
        }
}

double GrabCutImplementation::calculateBeta(const Mat &image) {
    double beta = 0;
    for (int row = 0; row < image.rows; row++)
        for (int col = 0; col < image.cols; col++) {
            Vec3b Pixel = image.at<Vec3b>(row, col);
            if (col >= 1) {
                auto &Left = image.at<Vec3b>(row, col - 1);
                beta += (Pixel - Left).dot(Pixel - Left);
            }
            if (col >= 1 && row >= 1) {
                auto &DownLeft = image.at<Vec3b>(row - 1, col - 1);
                beta += (Pixel - DownLeft).dot(Pixel - DownLeft);
            }
            if (row - 1 >= 0) {
                auto &Down = image.at<Vec3b>(row - 1, col);
                beta += (Pixel - Down).dot(Pixel - Down);
            }
            if (row - 1 >= 0 && col + 1 < image.cols) {
                auto &DownRight = image.at<Vec3b>(row - 1, col + 1);
                beta += (Pixel - DownRight).dot(Pixel - DownRight);
            }
        }
    if (beta > 0)
        beta = 1.0 / (2 * beta / (4 * image.cols * image.rows - 3 * image.cols - 3 * image.rows + 2));
    return beta;
}

void GrabCutImplementation::calculateNWeights(const Mat image, double beta, Mat left, Mat downLeft, Mat down,
                                              Mat downRight) {
    double sqrt2Gamma = sqrt(2) * gamma_;
    for (int row = 0; row < image.rows; row++)
        for (int col = 0; col < image.cols; col++) {
            Vec3b Pixel = image.at<Vec3b>(row, col);
            if (col - 1 >= 0) {
                auto &Left = image.at<Vec3b>(row, col - 1);
                left.at<double>(row, col) = gamma_ * exp(-beta * (Pixel - Left).dot(Pixel - Left));
            }
            if (col - 1 >= 0 && row - 1 >= 0) {
                auto &DownLeft = image.at<Vec3b>(row - 1, col - 1);
                downLeft.at<double>(row, col) = sqrt2Gamma * exp(-beta * (Pixel - DownLeft).dot(Pixel - DownLeft));
            }
            if (row - 1 >= 0) {
                auto &Down = image.at<Vec3b>(row - 1, col);
                down.at<double>(row, col) = gamma_ * exp(-beta * (Pixel - Down).dot(Pixel - Down));
            }
            if (row - 1 >= 0 && col + 1 < image.cols) {
                auto &DownRight = image.at<Vec3b>(row - 1, col + 1);
                downRight.at<double>(row, col) = sqrt2Gamma * exp(-beta * (Pixel - DownRight).dot(Pixel - DownRight));
            }
        }
}

void
GrabCutImplementation::constructNetwork(const Mat image, Mat mask, Mat left, Mat downLeft, Mat down, Mat downRight,
                                        GaussMixtureModel &backgroundGmm,
                                        GaussMixtureModel &foregroundGMM, Dinic *network) {
    const int Source = MAXN - 2;
    const int Target = MAXN - 1;
    network->init(Source, Target);
    for (int row = 0, nodeCounter = 0; row < image.rows; row++)
        for (int col = 0; col < image.cols; col++) {
            int currentNode = nodeCounter++;
            Vec3b Pixel = image.at<Vec3b>(row, col);
            double WeigtL = left.at<double>(row, col);
            double WeigtDL = downLeft.at<double>(row, col);
            double WeigtD = down.at<double>(row, col);
            double WeigtDR = downRight.at<double>(row, col);
            double sourceCapicity, targetCapicity;
            if (mask.at<uchar>(row, col) == GC_PR_BGD || mask.at<uchar>(row, col) == GC_PR_FGD) {
                double tmp[3] = {(double) Pixel[0], (double) Pixel[1], (double) Pixel[2]};
                sourceCapicity = -log(backgroundGmm.getProbability(tmp));
                targetCapicity = -log(foregroundGMM.getProbability(tmp));
            } else if (mask.at<uchar>(row, col) == GC_BGD) {
                sourceCapicity = 0;
                targetCapicity = lambda;
            } else {
                sourceCapicity = lambda;
                targetCapicity = 0;
            }
            network->addEdge(Source, currentNode, sourceCapicity);
            network->addEdge(currentNode, Target, targetCapicity);
            if (col - 1 >= 0)
                network->addEdge(currentNode, currentNode - 1, WeigtL);
            if (col - 1 >= 0 && row - 1 >= 0)
                network->addEdge(currentNode, currentNode - image.cols - 1, WeigtDL);
            if (row - 1 >= 0)
                network->addEdge(currentNode, currentNode - image.cols, WeigtD);
            if (col + 1 < image.cols && row - 1 >= 0)
                network->addEdge(currentNode, currentNode - image.cols + 1, WeigtDR);
        }
}

void GrabCutImplementation::grabCut(cv::InputArray imageInputArray, cv::InputOutputArray maskIOArray, int iterCount) {
    payloadImage = imageInputArray.getMat();
    Mat left = Mat::zeros(payloadImage.size(), CV_64F);
    Mat downLeft = Mat::zeros(payloadImage.size(), CV_64F);
    Mat down = Mat::zeros(payloadImage.size(), CV_64F);
    Mat downRight = Mat::zeros(payloadImage.size(), CV_64F);
    double beta = calculateBeta(payloadImage);
    payloadMask = maskIOArray.getMat();
    while (iterCount--) {
        auto *network = new Dinic();
        vector<double> backgroundPixelArray, foregroundPixelArray;
        GaussMixtureModel backgroundGmm, foregroundGmm;
        for (int row = 0; row < payloadMask.rows; row++)
            for (int col = 0; col < payloadMask.cols; col++) {
                Vec3b tmp = payloadImage.at<Vec3b>(row, col);
                if (payloadMask.at<uchar>(row, col) & 1) {
                    foregroundPixelArray.push_back(tmp[0]);
                    foregroundPixelArray.push_back(tmp[1]);
                    foregroundPixelArray.push_back(tmp[2]);
                } else {
                    backgroundPixelArray.push_back(tmp[0]);
                    backgroundPixelArray.push_back(tmp[1]);
                    backgroundPixelArray.push_back(tmp[2]);
                }
            }
        auto *data = new double[foregroundPixelArray.size()];
        for (int i = 0; i < foregroundPixelArray.size(); i++)
            data[i] = foregroundPixelArray[i];
        foregroundGmm.train(data, foregroundPixelArray.size() / 3);
        delete[]data;
        data = new double[backgroundPixelArray.size()];
        for (int i = 0; i < backgroundPixelArray.size(); i++)
            data[i] = backgroundPixelArray[i];
        backgroundGmm.train(data, backgroundPixelArray.size() / 3);
        calculateNWeights(payloadImage, beta, left, downLeft, down, downRight);
        constructNetwork(payloadImage, payloadMask, left, downLeft, down, downRight, backgroundGmm, foregroundGmm,
                         network);
        network->flow();
        for (int row = 0; row < payloadMask.rows; row++)
            for (int col = 0; col < payloadMask.cols; col++)
                if (payloadMask.at<uchar>(row, col) != GC_FGD)
                    if (network->sourceSet(row * payloadMask.cols + col))
                        payloadMask.at<uchar>(row, col) = GC_PR_FGD;
                    else
                        payloadMask.at<uchar>(row, col) = GC_BGD;
    }
}

void GrabCutImplementation::doBorderMatting() {
    Mat result = Mat(payloadImage.size(), payloadImage.type());
    payloadImage.copyTo(result);
    for (int row = 0; row < result.rows; row++)
        for (int col = 0; col < result.cols; col++) {
            if (payloadMask.at<uchar>(row, col) == 0)
                result.at<Vec3b>(row, col) = Vec3b(255, 255, 255);
        }
    imwrite("whiteBackground.jpg", result);
    borderMatting.init(payloadImage, payloadMask);
    borderMatting.run();
}